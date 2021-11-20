// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#include <algorithm>
#include <functional>
#include <tuple>
#include <type_traits>

#include <range/v3/algorithm.hpp>

#include <common/bit_cast.h>
#include <common/bit_util.h>
#include <exception.h>
#include <frontend/ir/ir_emitter.h>
#include <frontend/ir/value.h>
#include "passes.h"

namespace Shader::Optimization {
namespace {
// Metaprogramming stuff to get arguments information out of a lambda
template <typename Func>
struct LambdaTraits : LambdaTraits<decltype(&std::remove_reference_t<Func>::operator())> {};

template <typename ReturnType, typename LambdaType, typename... Args>
struct LambdaTraits<ReturnType (LambdaType::*)(Args...) const> {
    template <size_t I>
    using ArgType = std::tuple_element_t<I, std::tuple<Args...>>;

    static constexpr size_t NUM_ARGS{sizeof...(Args)};
};

template <typename T>
[[nodiscard]] T Arg(const IR::Value& value) {
    if constexpr (std::is_same_v<T, bool>) {
        return value.U1();
    } else if constexpr (std::is_same_v<T, u32>) {
        return value.U32();
    } else if constexpr (std::is_same_v<T, s32>) {
        return static_cast<s32>(value.U32());
    } else if constexpr (std::is_same_v<T, f32>) {
        return value.F32();
    } else if constexpr (std::is_same_v<T, u64>) {
        return value.U64();
    }
}

template <typename T, typename ImmFn>
bool FoldCommutative(IR::Inst& inst, ImmFn&& imm_fn) {
    const IR::Value lhs{inst.Arg(0)};
    const IR::Value rhs{inst.Arg(1)};

    const bool is_lhs_immediate{lhs.IsImmediate()};
    const bool is_rhs_immediate{rhs.IsImmediate()};

    if (is_lhs_immediate && is_rhs_immediate) {
        const auto result{imm_fn(Arg<T>(lhs), Arg<T>(rhs))};
        inst.ReplaceUsesWith(IR::Value{result});
        return false;
    }
    if (is_lhs_immediate && !is_rhs_immediate) {
        IR::Inst* const rhs_inst{rhs.InstRecursive()};
        if (rhs_inst->GetOpcode() == inst.GetOpcode() && rhs_inst->Arg(1).IsImmediate()) {
            const auto combined{imm_fn(Arg<T>(lhs), Arg<T>(rhs_inst->Arg(1)))};
            inst.SetArg(0, rhs_inst->Arg(0));
            inst.SetArg(1, IR::Value{combined});
        } else {
            // Normalize
            inst.SetArg(0, rhs);
            inst.SetArg(1, lhs);
        }
    }
    if (!is_lhs_immediate && is_rhs_immediate) {
        const IR::Inst* const lhs_inst{lhs.InstRecursive()};
        if (lhs_inst->GetOpcode() == inst.GetOpcode() && lhs_inst->Arg(1).IsImmediate()) {
            const auto combined{imm_fn(Arg<T>(rhs), Arg<T>(lhs_inst->Arg(1)))};
            inst.SetArg(0, lhs_inst->Arg(0));
            inst.SetArg(1, IR::Value{combined});
        }
    }
    return true;
}

template <typename Func>
bool FoldWhenAllImmediates(IR::Inst& inst, Func&& func) {
    if (!inst.AreAllArgsImmediates() || inst.HasAssociatedPseudoOperation()) {
        return false;
    }
    using Indices = std::make_index_sequence<LambdaTraits<decltype(func)>::NUM_ARGS>;
    inst.ReplaceUsesWith(EvalImmediates(inst, func, Indices{}));
    return true;
}

/// Return true when all values in a range are equal
template <typename Range>
bool AreEqual(const Range& range) {
    auto resolver{[](const auto& value) { return value.Resolve(); }};
    auto equal{[](const IR::Value& lhs, const IR::Value& rhs) {
        if (lhs == rhs) {
            return true;
        }
        // Not equal, but try to match if they read the same constant buffer
        if (!lhs.IsImmediate() && !rhs.IsImmediate() &&
            lhs.Inst()->GetOpcode() == IR::Opcode::GetCbufU32 &&
            rhs.Inst()->GetOpcode() == IR::Opcode::GetCbufU32 &&
            lhs.Inst()->Arg(0) == rhs.Inst()->Arg(0) && lhs.Inst()->Arg(1) == rhs.Inst()->Arg(1)) {
            return true;
        }
        return false;
    }};
    return ranges::adjacent_find(range, std::not_fn(equal), resolver) == std::end(range);
}

void FoldGetRegister(IR::Inst& inst) {
    if (inst.Arg(0).Reg() == IR::Reg::RZ) {
        inst.ReplaceUsesWith(IR::Value{u32{0}});
    }
}

void FoldGetPred(IR::Inst& inst) {
    if (inst.Arg(0).Pred() == IR::Pred::PT) {
        inst.ReplaceUsesWith(IR::Value{true});
    }
}

/// Replaces the XMAD pattern generated by an integer FMA
bool FoldXmadMultiplyAdd(IR::Block& block, IR::Inst& inst) {
    /*
     * We are looking for this specific pattern:
     *   %6      = BitFieldUExtract %op_b, #0, #16
     *   %7      = BitFieldUExtract %op_a', #16, #16
     *   %8      = IMul32 %6, %7
     *   %10     = BitFieldUExtract %op_a', #0, #16
     *   %11     = BitFieldInsert %8, %10, #16, #16
     *   %15     = BitFieldUExtract %op_b, #0, #16
     *   %16     = BitFieldUExtract %op_a, #0, #16
     *   %17     = IMul32 %15, %16
     *   %18     = IAdd32 %17, %op_c
     *   %22     = BitFieldUExtract %op_b, #16, #16
     *   %23     = BitFieldUExtract %11, #16, #16
     *   %24     = IMul32 %22, %23
     *   %25     = ShiftLeftLogical32 %24, #16
     *   %26     = ShiftLeftLogical32 %11, #16
     *   %27     = IAdd32 %26, %18
     *   %result = IAdd32 %25, %27
     *
     * And replace it with:
     *  %temp   = IMul32 %op_a, %op_b
     *  %result = IAdd32 %temp, %op_c
     *
     * This optimization has been proven safe by Nvidia's compiler logic being reversed.
     * (If Nvidia generates this code from 'fma(a, b, c)', we can do the same in the reverse order.)
     */
    const IR::Value zero{0u};
    const IR::Value sixteen{16u};
    IR::Inst* const _25{inst.Arg(0).TryInstRecursive()};
    IR::Inst* const _27{inst.Arg(1).TryInstRecursive()};
    if (!_25 || !_27) {
        return false;
    }
    if (_27->GetOpcode() != IR::Opcode::IAdd32) {
        return false;
    }
    if (_25->GetOpcode() != IR::Opcode::ShiftLeftLogical32 || _25->Arg(1) != sixteen) {
        return false;
    }
    IR::Inst* const _24{_25->Arg(0).TryInstRecursive()};
    if (!_24 || _24->GetOpcode() != IR::Opcode::IMul32) {
        return false;
    }
    IR::Inst* const _22{_24->Arg(0).TryInstRecursive()};
    IR::Inst* const _23{_24->Arg(1).TryInstRecursive()};
    if (!_22 || !_23) {
        return false;
    }
    if (_22->GetOpcode() != IR::Opcode::BitFieldUExtract) {
        return false;
    }
    if (_23->GetOpcode() != IR::Opcode::BitFieldUExtract) {
        return false;
    }
    if (_22->Arg(1) != sixteen || _22->Arg(2) != sixteen) {
        return false;
    }
    if (_23->Arg(1) != sixteen || _23->Arg(2) != sixteen) {
        return false;
    }
    IR::Inst* const _11{_23->Arg(0).TryInstRecursive()};
    if (!_11 || _11->GetOpcode() != IR::Opcode::BitFieldInsert) {
        return false;
    }
    if (_11->Arg(2) != sixteen || _11->Arg(3) != sixteen) {
        return false;
    }
    IR::Inst* const _8{_11->Arg(0).TryInstRecursive()};
    IR::Inst* const _10{_11->Arg(1).TryInstRecursive()};
    if (!_8 || !_10) {
        return false;
    }
    if (_8->GetOpcode() != IR::Opcode::IMul32) {
        return false;
    }
    if (_10->GetOpcode() != IR::Opcode::BitFieldUExtract) {
        return false;
    }
    IR::Inst* const _6{_8->Arg(0).TryInstRecursive()};
    IR::Inst* const _7{_8->Arg(1).TryInstRecursive()};
    if (!_6 || !_7) {
        return false;
    }
    if (_6->GetOpcode() != IR::Opcode::BitFieldUExtract) {
        return false;
    }
    if (_7->GetOpcode() != IR::Opcode::BitFieldUExtract) {
        return false;
    }
    if (_6->Arg(1) != zero || _6->Arg(2) != sixteen) {
        return false;
    }
    if (_7->Arg(1) != sixteen || _7->Arg(2) != sixteen) {
        return false;
    }
    IR::Inst* const _26{_27->Arg(0).TryInstRecursive()};
    IR::Inst* const _18{_27->Arg(1).TryInstRecursive()};
    if (!_26 || !_18) {
        return false;
    }
    if (_26->GetOpcode() != IR::Opcode::ShiftLeftLogical32 || _26->Arg(1) != sixteen) {
        return false;
    }
    if (_26->Arg(0).InstRecursive() != _11) {
        return false;
    }
    if (_18->GetOpcode() != IR::Opcode::IAdd32) {
        return false;
    }
    IR::Inst* const _17{_18->Arg(0).TryInstRecursive()};
    if (!_17 || _17->GetOpcode() != IR::Opcode::IMul32) {
        return false;
    }
    IR::Inst* const _15{_17->Arg(0).TryInstRecursive()};
    IR::Inst* const _16{_17->Arg(1).TryInstRecursive()};
    if (!_15 || !_16) {
        return false;
    }
    if (_15->GetOpcode() != IR::Opcode::BitFieldUExtract) {
        return false;
    }
    if (_16->GetOpcode() != IR::Opcode::BitFieldUExtract) {
        return false;
    }
    if (_15->Arg(1) != zero || _16->Arg(1) != zero || _10->Arg(1) != zero) {
        return false;
    }
    if (_15->Arg(2) != sixteen || _16->Arg(2) != sixteen || _10->Arg(2) != sixteen) {
        return false;
    }
    const std::array<IR::Value, 3> op_as{
        _7->Arg(0).Resolve(),
        _16->Arg(0).Resolve(),
        _10->Arg(0).Resolve(),
    };
    const std::array<IR::Value, 3> op_bs{
        _22->Arg(0).Resolve(),
        _6->Arg(0).Resolve(),
        _15->Arg(0).Resolve(),
    };
    const IR::U32 op_c{_18->Arg(1)};
    if (!AreEqual(op_as) || !AreEqual(op_bs)) {
        return false;
    }
    IR::IREmitter ir{block, IR::Block::InstructionList::s_iterator_to(inst)};
    inst.ReplaceUsesWith(ir.IAdd(ir.IMul(IR::U32{op_as[0]}, IR::U32{op_bs[1]}), op_c));
    return true;
}

/// Replaces the pattern generated by two XMAD multiplications
bool FoldXmadMultiply(IR::Block& block, IR::Inst& inst) {
    /*
     * We are looking for this pattern:
     *   %rhs_bfe = BitFieldUExtract %factor_a, #0, #16
     *   %rhs_mul = IMul32 %rhs_bfe, %factor_b
     *   %lhs_bfe = BitFieldUExtract %factor_a, #16, #16
     *   %rhs_mul = IMul32 %lhs_bfe, %factor_b
     *   %lhs_shl = ShiftLeftLogical32 %rhs_mul, #16
     *   %result  = IAdd32 %lhs_shl, %rhs_mul
     *
     * And replacing it with
     *   %result  = IMul32 %factor_a, %factor_b
     *
     * This optimization has been proven safe by LLVM and MSVC.
     */
    IR::Inst* const lhs_shl{inst.Arg(0).TryInstRecursive()};
    IR::Inst* const rhs_mul{inst.Arg(1).TryInstRecursive()};
    if (!lhs_shl || !rhs_mul) {
        return false;
    }
    if (lhs_shl->GetOpcode() != IR::Opcode::ShiftLeftLogical32 ||
        lhs_shl->Arg(1) != IR::Value{16U}) {
        return false;
    }
    IR::Inst* const lhs_mul{lhs_shl->Arg(0).TryInstRecursive()};
    if (!lhs_mul) {
        return false;
    }
    if (lhs_mul->GetOpcode() != IR::Opcode::IMul32 || rhs_mul->GetOpcode() != IR::Opcode::IMul32) {
        return false;
    }
    const IR::U32 factor_b{lhs_mul->Arg(1)};
    if (factor_b.Resolve() != rhs_mul->Arg(1).Resolve()) {
        return false;
    }
    IR::Inst* const lhs_bfe{lhs_mul->Arg(0).TryInstRecursive()};
    IR::Inst* const rhs_bfe{rhs_mul->Arg(0).TryInstRecursive()};
    if (!lhs_bfe || !rhs_bfe) {
        return false;
    }
    if (lhs_bfe->GetOpcode() != IR::Opcode::BitFieldUExtract) {
        return false;
    }
    if (rhs_bfe->GetOpcode() != IR::Opcode::BitFieldUExtract) {
        return false;
    }
    if (lhs_bfe->Arg(1) != IR::Value{16U} || lhs_bfe->Arg(2) != IR::Value{16U}) {
        return false;
    }
    if (rhs_bfe->Arg(1) != IR::Value{0U} || rhs_bfe->Arg(2) != IR::Value{16U}) {
        return false;
    }
    const IR::U32 factor_a{lhs_bfe->Arg(0)};
    if (factor_a.Resolve() != rhs_bfe->Arg(0).Resolve()) {
        return false;
    }
    IR::IREmitter ir{block, IR::Block::InstructionList::s_iterator_to(inst)};
    inst.ReplaceUsesWith(ir.IMul(factor_a, factor_b));
    return true;
}

template <typename T>
void FoldAdd(IR::Block& block, IR::Inst& inst) {
    if (inst.HasAssociatedPseudoOperation()) {
        return;
    }
    if (!FoldCommutative<T>(inst, [](T a, T b) { return a + b; })) {
        return;
    }
    const IR::Value rhs{inst.Arg(1)};
    if (rhs.IsImmediate() && Arg<T>(rhs) == 0) {
        inst.ReplaceUsesWith(inst.Arg(0));
        return;
    }
    if constexpr (std::is_same_v<T, u32>) {
        if (FoldXmadMultiply(block, inst)) {
            return;
        }
        if (FoldXmadMultiplyAdd(block, inst)) {
            return;
        }
    }
}

void FoldISub32(IR::Inst& inst) {
    if (FoldWhenAllImmediates(inst, [](u32 a, u32 b) { return a - b; })) {
        return;
    }
    if (inst.Arg(0).IsImmediate() || inst.Arg(1).IsImmediate()) {
        return;
    }
    // ISub32 is generally used to subtract two constant buffers, compare and replace this with
    // zero if they equal.
    const auto equal_cbuf{[](IR::Inst* a, IR::Inst* b) {
        return a->GetOpcode() == IR::Opcode::GetCbufU32 &&
               b->GetOpcode() == IR::Opcode::GetCbufU32 && a->Arg(0) == b->Arg(0) &&
               a->Arg(1) == b->Arg(1);
    }};
    IR::Inst* op_a{inst.Arg(0).InstRecursive()};
    IR::Inst* op_b{inst.Arg(1).InstRecursive()};
    if (equal_cbuf(op_a, op_b)) {
        inst.ReplaceUsesWith(IR::Value{u32{0}});
        return;
    }
    // It's also possible a value is being added to a cbuf and then subtracted
    if (op_b->GetOpcode() == IR::Opcode::IAdd32) {
        // Canonicalize local variables to simplify the following logic
        std::swap(op_a, op_b);
    }
    if (op_b->GetOpcode() != IR::Opcode::GetCbufU32) {
        return;
    }
    IR::Inst* const inst_cbuf{op_b};
    if (op_a->GetOpcode() != IR::Opcode::IAdd32) {
        return;
    }
    IR::Value add_op_a{op_a->Arg(0)};
    IR::Value add_op_b{op_a->Arg(1)};
    if (add_op_b.IsImmediate()) {
        // Canonicalize
        std::swap(add_op_a, add_op_b);
    }
    if (add_op_b.IsImmediate()) {
        return;
    }
    IR::Inst* const add_cbuf{add_op_b.InstRecursive()};
    if (equal_cbuf(add_cbuf, inst_cbuf)) {
        inst.ReplaceUsesWith(add_op_a);
    }
}

void FoldSelect(IR::Inst& inst) {
    const IR::Value cond{inst.Arg(0)};
    if (cond.IsImmediate()) {
        inst.ReplaceUsesWith(cond.U1() ? inst.Arg(1) : inst.Arg(2));
    }
}

void FoldFPMul32(IR::Inst& inst) {
    const auto control{inst.Flags<IR::FpControl>()};
    if (control.no_contraction) {
        return;
    }
    // Fold interpolation operations
    const IR::Value lhs_value{inst.Arg(0)};
    const IR::Value rhs_value{inst.Arg(1)};
    if (lhs_value.IsImmediate() || rhs_value.IsImmediate()) {
        return;
    }
    IR::Inst* const lhs_op{lhs_value.InstRecursive()};
    IR::Inst* const rhs_op{rhs_value.InstRecursive()};
    if (lhs_op->GetOpcode() != IR::Opcode::FPMul32 ||
        rhs_op->GetOpcode() != IR::Opcode::FPRecip32) {
        return;
    }
    const IR::Value recip_source{rhs_op->Arg(0)};
    const IR::Value lhs_mul_source{lhs_op->Arg(1).Resolve()};
    if (recip_source.IsImmediate() || lhs_mul_source.IsImmediate()) {
        return;
    }
    IR::Inst* const attr_a{recip_source.InstRecursive()};
    IR::Inst* const attr_b{lhs_mul_source.InstRecursive()};
    if (attr_a->GetOpcode() != IR::Opcode::GetAttribute ||
        attr_b->GetOpcode() != IR::Opcode::GetAttribute) {
        return;
    }
    if (attr_a->Arg(0).Attribute() == attr_b->Arg(0).Attribute()) {
        inst.ReplaceUsesWith(lhs_op->Arg(0));
    }
}

void FoldLogicalAnd(IR::Inst& inst) {
    if (!FoldCommutative<bool>(inst, [](bool a, bool b) { return a && b; })) {
        return;
    }
    const IR::Value rhs{inst.Arg(1)};
    if (rhs.IsImmediate()) {
        if (rhs.U1()) {
            inst.ReplaceUsesWith(inst.Arg(0));
        } else {
            inst.ReplaceUsesWith(IR::Value{false});
        }
    }
}

void FoldLogicalOr(IR::Inst& inst) {
    if (!FoldCommutative<bool>(inst, [](bool a, bool b) { return a || b; })) {
        return;
    }
    const IR::Value rhs{inst.Arg(1)};
    if (rhs.IsImmediate()) {
        if (rhs.U1()) {
            inst.ReplaceUsesWith(IR::Value{true});
        } else {
            inst.ReplaceUsesWith(inst.Arg(0));
        }
    }
}

void FoldLogicalNot(IR::Inst& inst) {
    const IR::U1 value{inst.Arg(0)};
    if (value.IsImmediate()) {
        inst.ReplaceUsesWith(IR::Value{!value.U1()});
        return;
    }
    IR::Inst* const arg{value.InstRecursive()};
    if (arg->GetOpcode() == IR::Opcode::LogicalNot) {
        inst.ReplaceUsesWith(arg->Arg(0));
    }
}

template <IR::Opcode op, typename Dest, typename Source>
void FoldBitCast(IR::Inst& inst, IR::Opcode reverse) {
    const IR::Value value{inst.Arg(0)};
    if (value.IsImmediate()) {
        inst.ReplaceUsesWith(IR::Value{Common::BitCast<Dest>(Arg<Source>(value))});
        return;
    }
    IR::Inst* const arg_inst{value.InstRecursive()};
    if (arg_inst->GetOpcode() == reverse) {
        inst.ReplaceUsesWith(arg_inst->Arg(0));
        return;
    }
    if constexpr (op == IR::Opcode::BitCastF32U32) {
        if (arg_inst->GetOpcode() == IR::Opcode::GetCbufU32) {
            // Replace the bitcast with a typed constant buffer read
            inst.ReplaceOpcode(IR::Opcode::GetCbufF32);
            inst.SetArg(0, arg_inst->Arg(0));
            inst.SetArg(1, arg_inst->Arg(1));
            return;
        }
    }
}

void FoldInverseFunc(IR::Inst& inst, IR::Opcode reverse) {
    const IR::Value value{inst.Arg(0)};
    if (value.IsImmediate()) {
        return;
    }
    IR::Inst* const arg_inst{value.InstRecursive()};
    if (arg_inst->GetOpcode() == reverse) {
        inst.ReplaceUsesWith(arg_inst->Arg(0));
        return;
    }
}

template <typename Func, size_t... I>
IR::Value EvalImmediates(const IR::Inst& inst, Func&& func, std::index_sequence<I...>) {
    using Traits = LambdaTraits<decltype(func)>;
    return IR::Value{func(Arg<typename Traits::template ArgType<I>>(inst.Arg(I))...)};
}

std::optional<IR::Value> FoldCompositeExtractImpl(IR::Value inst_value, IR::Opcode insert,
                                                  IR::Opcode construct, u32 first_index) {
    IR::Inst* const inst{inst_value.InstRecursive()};
    if (inst->GetOpcode() == construct) {
        return inst->Arg(first_index);
    }
    if (inst->GetOpcode() != insert) {
        return std::nullopt;
    }
    IR::Value value_index{inst->Arg(2)};
    if (!value_index.IsImmediate()) {
        return std::nullopt;
    }
    const u32 second_index{value_index.U32()};
    if (first_index != second_index) {
        IR::Value value_composite{inst->Arg(0)};
        if (value_composite.IsImmediate()) {
            return std::nullopt;
        }
        return FoldCompositeExtractImpl(value_composite, insert, construct, first_index);
    }
    return inst->Arg(1);
}

void FoldCompositeExtract(IR::Inst& inst, IR::Opcode construct, IR::Opcode insert) {
    const IR::Value value_1{inst.Arg(0)};
    const IR::Value value_2{inst.Arg(1)};
    if (value_1.IsImmediate()) {
        return;
    }
    if (!value_2.IsImmediate()) {
        return;
    }
    const u32 first_index{value_2.U32()};
    const std::optional result{FoldCompositeExtractImpl(value_1, insert, construct, first_index)};
    if (!result) {
        return;
    }
    inst.ReplaceUsesWith(*result);
}

IR::Value GetThroughCast(IR::Value value, IR::Opcode expected_cast) {
    if (value.IsImmediate()) {
        return value;
    }
    IR::Inst* const inst{value.InstRecursive()};
    if (inst->GetOpcode() == expected_cast) {
        return inst->Arg(0).Resolve();
    }
    return value;
}

void FoldFSwizzleAdd(IR::Block& block, IR::Inst& inst) {
    const IR::Value swizzle{inst.Arg(2)};
    if (!swizzle.IsImmediate()) {
        return;
    }
    const IR::Value value_1{GetThroughCast(inst.Arg(0).Resolve(), IR::Opcode::BitCastF32U32)};
    const IR::Value value_2{GetThroughCast(inst.Arg(1).Resolve(), IR::Opcode::BitCastF32U32)};
    if (value_1.IsImmediate()) {
        return;
    }
    const u32 swizzle_value{swizzle.U32()};
    if (swizzle_value != 0x99 && swizzle_value != 0xA5) {
        return;
    }
    IR::Inst* const inst2{value_1.InstRecursive()};
    if (inst2->GetOpcode() != IR::Opcode::ShuffleButterfly) {
        return;
    }
    const IR::Value value_3{GetThroughCast(inst2->Arg(0).Resolve(), IR::Opcode::BitCastU32F32)};
    if (value_2 != value_3) {
        return;
    }
    const IR::Value index{inst2->Arg(1)};
    const IR::Value clamp{inst2->Arg(2)};
    const IR::Value segmentation_mask{inst2->Arg(3)};
    if (!index.IsImmediate() || !clamp.IsImmediate() || !segmentation_mask.IsImmediate()) {
        return;
    }
    if (clamp.U32() != 3 || segmentation_mask.U32() != 28) {
        return;
    }
    if (swizzle_value == 0x99) {
        // DPdxFine
        if (index.U32() == 1) {
            IR::IREmitter ir{block, IR::Block::InstructionList::s_iterator_to(inst)};
            inst.ReplaceUsesWith(ir.DPdxFine(IR::F32{inst.Arg(1)}));
        }
    } else if (swizzle_value == 0xA5) {
        // DPdyFine
        if (index.U32() == 2) {
            IR::IREmitter ir{block, IR::Block::InstructionList::s_iterator_to(inst)};
            inst.ReplaceUsesWith(ir.DPdyFine(IR::F32{inst.Arg(1)}));
        }
    }
}

void ConstantPropagation(IR::Block& block, IR::Inst& inst) {
    switch (inst.GetOpcode()) {
    case IR::Opcode::GetRegister:
        return FoldGetRegister(inst);
    case IR::Opcode::GetPred:
        return FoldGetPred(inst);
    case IR::Opcode::IAdd32:
        return FoldAdd<u32>(block, inst);
    case IR::Opcode::ISub32:
        return FoldISub32(inst);
    case IR::Opcode::IMul32:
        FoldWhenAllImmediates(inst, [](u32 a, u32 b) { return a * b; });
        return;
    case IR::Opcode::ShiftRightArithmetic32:
        FoldWhenAllImmediates(inst, [](s32 a, s32 b) { return static_cast<u32>(a >> b); });
        return;
    case IR::Opcode::BitCastF32U32:
        return FoldBitCast<IR::Opcode::BitCastF32U32, f32, u32>(inst, IR::Opcode::BitCastU32F32);
    case IR::Opcode::BitCastU32F32:
        return FoldBitCast<IR::Opcode::BitCastU32F32, u32, f32>(inst, IR::Opcode::BitCastF32U32);
    case IR::Opcode::IAdd64:
        return FoldAdd<u64>(block, inst);
    case IR::Opcode::PackHalf2x16:
        return FoldInverseFunc(inst, IR::Opcode::UnpackHalf2x16);
    case IR::Opcode::UnpackHalf2x16:
        return FoldInverseFunc(inst, IR::Opcode::PackHalf2x16);
    case IR::Opcode::PackFloat2x16:
        return FoldInverseFunc(inst, IR::Opcode::UnpackFloat2x16);
    case IR::Opcode::UnpackFloat2x16:
        return FoldInverseFunc(inst, IR::Opcode::PackFloat2x16);
    case IR::Opcode::SelectU1:
    case IR::Opcode::SelectU8:
    case IR::Opcode::SelectU16:
    case IR::Opcode::SelectU32:
    case IR::Opcode::SelectU64:
    case IR::Opcode::SelectF16:
    case IR::Opcode::SelectF32:
    case IR::Opcode::SelectF64:
        return FoldSelect(inst);
    case IR::Opcode::FPMul32:
        return FoldFPMul32(inst);
    case IR::Opcode::LogicalAnd:
        return FoldLogicalAnd(inst);
    case IR::Opcode::LogicalOr:
        return FoldLogicalOr(inst);
    case IR::Opcode::LogicalNot:
        return FoldLogicalNot(inst);
    case IR::Opcode::SLessThan:
        FoldWhenAllImmediates(inst, [](s32 a, s32 b) { return a < b; });
        return;
    case IR::Opcode::ULessThan:
        FoldWhenAllImmediates(inst, [](u32 a, u32 b) { return a < b; });
        return;
    case IR::Opcode::SLessThanEqual:
        FoldWhenAllImmediates(inst, [](s32 a, s32 b) { return a <= b; });
        return;
    case IR::Opcode::ULessThanEqual:
        FoldWhenAllImmediates(inst, [](u32 a, u32 b) { return a <= b; });
        return;
    case IR::Opcode::SGreaterThan:
        FoldWhenAllImmediates(inst, [](s32 a, s32 b) { return a > b; });
        return;
    case IR::Opcode::UGreaterThan:
        FoldWhenAllImmediates(inst, [](u32 a, u32 b) { return a > b; });
        return;
    case IR::Opcode::SGreaterThanEqual:
        FoldWhenAllImmediates(inst, [](s32 a, s32 b) { return a >= b; });
        return;
    case IR::Opcode::UGreaterThanEqual:
        FoldWhenAllImmediates(inst, [](u32 a, u32 b) { return a >= b; });
        return;
    case IR::Opcode::IEqual:
        FoldWhenAllImmediates(inst, [](u32 a, u32 b) { return a == b; });
        return;
    case IR::Opcode::INotEqual:
        FoldWhenAllImmediates(inst, [](u32 a, u32 b) { return a != b; });
        return;
    case IR::Opcode::BitwiseAnd32:
        FoldWhenAllImmediates(inst, [](u32 a, u32 b) { return a & b; });
        return;
    case IR::Opcode::BitwiseOr32:
        FoldWhenAllImmediates(inst, [](u32 a, u32 b) { return a | b; });
        return;
    case IR::Opcode::BitwiseXor32:
        FoldWhenAllImmediates(inst, [](u32 a, u32 b) { return a ^ b; });
        return;
    case IR::Opcode::BitFieldUExtract:
        FoldWhenAllImmediates(inst, [](u32 base, u32 shift, u32 count) {
            if (static_cast<size_t>(shift) + static_cast<size_t>(count) > 32) {
                throw LogicError("Undefined result in {}({}, {}, {})", IR::Opcode::BitFieldUExtract,
                                 base, shift, count);
            }
            return (base >> shift) & ((1U << count) - 1);
        });
        return;
    case IR::Opcode::BitFieldSExtract:
        FoldWhenAllImmediates(inst, [](s32 base, u32 shift, u32 count) {
            const size_t back_shift{static_cast<size_t>(shift) + static_cast<size_t>(count)};
            const size_t left_shift{32 - back_shift};
            const size_t right_shift{static_cast<size_t>(32 - count)};
            if (back_shift > 32 || left_shift >= 32 || right_shift >= 32) {
                throw LogicError("Undefined result in {}({}, {}, {})", IR::Opcode::BitFieldSExtract,
                                 base, shift, count);
            }
            return static_cast<u32>((base << left_shift) >> right_shift);
        });
        return;
    case IR::Opcode::BitFieldInsert:
        FoldWhenAllImmediates(inst, [](u32 base, u32 insert, u32 offset, u32 bits) {
            if (bits >= 32 || offset >= 32) {
                throw LogicError("Undefined result in {}({}, {}, {}, {})",
                                 IR::Opcode::BitFieldInsert, base, insert, offset, bits);
            }
            return (base & ~(~(~0u << bits) << offset)) | (insert << offset);
        });
        return;
    case IR::Opcode::CompositeExtractU32x2:
        return FoldCompositeExtract(inst, IR::Opcode::CompositeConstructU32x2,
                                    IR::Opcode::CompositeInsertU32x2);
    case IR::Opcode::CompositeExtractU32x3:
        return FoldCompositeExtract(inst, IR::Opcode::CompositeConstructU32x3,
                                    IR::Opcode::CompositeInsertU32x3);
    case IR::Opcode::CompositeExtractU32x4:
        return FoldCompositeExtract(inst, IR::Opcode::CompositeConstructU32x4,
                                    IR::Opcode::CompositeInsertU32x4);
    case IR::Opcode::CompositeExtractF32x2:
        return FoldCompositeExtract(inst, IR::Opcode::CompositeConstructF32x2,
                                    IR::Opcode::CompositeInsertF32x2);
    case IR::Opcode::CompositeExtractF32x3:
        return FoldCompositeExtract(inst, IR::Opcode::CompositeConstructF32x3,
                                    IR::Opcode::CompositeInsertF32x3);
    case IR::Opcode::CompositeExtractF32x4:
        return FoldCompositeExtract(inst, IR::Opcode::CompositeConstructF32x4,
                                    IR::Opcode::CompositeInsertF32x4);
    case IR::Opcode::CompositeExtractF16x2:
        return FoldCompositeExtract(inst, IR::Opcode::CompositeConstructF16x2,
                                    IR::Opcode::CompositeInsertF16x2);
    case IR::Opcode::CompositeExtractF16x3:
        return FoldCompositeExtract(inst, IR::Opcode::CompositeConstructF16x3,
                                    IR::Opcode::CompositeInsertF16x3);
    case IR::Opcode::CompositeExtractF16x4:
        return FoldCompositeExtract(inst, IR::Opcode::CompositeConstructF16x4,
                                    IR::Opcode::CompositeInsertF16x4);
    case IR::Opcode::FSwizzleAdd:
        return FoldFSwizzleAdd(block, inst);
    default:
        break;
    }
}
} // Anonymous namespace

void ConstantPropagationPass(IR::Program& program) {
    const auto end{program.post_order_blocks.rend()};
    for (auto it = program.post_order_blocks.rbegin(); it != end; ++it) {
        IR::Block* const block{*it};
        for (IR::Inst& inst : block->Instructions()) {
            ConstantPropagation(*block, inst);
        }
    }
}

} // namespace Shader::Optimization
