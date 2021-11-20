// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#include "emit_spirv.h"
#include "emit_spirv_instructions.h"

namespace Shader::Backend::SPIRV {

Id EmitUndefU1(EmitContext& ctx) {
    return ctx.OpUndef(ctx.U1);
}

Id EmitUndefU8(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitUndefU16(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

Id EmitUndefU32(EmitContext& ctx) {
    return ctx.OpUndef(ctx.U32[1]);
}

Id EmitUndefU64(EmitContext&) {
    throw NotImplementedException("SPIR-V Instruction");
}

} // namespace Shader::Backend::SPIRV
