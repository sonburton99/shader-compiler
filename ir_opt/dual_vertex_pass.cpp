// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#include <frontend/ir/ir_emitter.h>
#include "passes.h"

namespace Shader::Optimization {

void VertexATransformPass(IR::Program& program) {
    for (IR::Block* const block : program.blocks) {
        for (IR::Inst& inst : block->Instructions()) {
            if (inst.GetOpcode() == IR::Opcode::Epilogue) {
                return inst.Invalidate();
            }
        }
    }
}

void VertexBTransformPass(IR::Program& program) {
    for (IR::Block* const block : program.blocks) {
        for (IR::Inst& inst : block->Instructions()) {
            if (inst.GetOpcode() == IR::Opcode::Prologue) {
                return inst.Invalidate();
            }
        }
    }
}

} // namespace Shader::Optimization
