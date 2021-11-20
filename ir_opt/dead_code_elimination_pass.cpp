// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#include <frontend/ir/basic_block.h>
#include <frontend/ir/value.h>
#include "passes.h"

namespace Shader::Optimization {

void DeadCodeEliminationPass(IR::Program& program) {
    // We iterate over the instructions in reverse order.
    // This is because removing an instruction reduces the number of uses for earlier instructions.
    for (IR::Block* const block : program.post_order_blocks) {
        auto it{block->end()};
        while (it != block->begin()) {
            --it;
            if (!it->HasUses() && !it->MayHaveSideEffects()) {
                it->Invalidate();
                it = block->Instructions().erase(it);
            }
        }
    }
}

} // namespace Shader::Optimization
