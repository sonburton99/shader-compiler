// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#include <shader_compiler/exception.h>
#include <shader_compiler/frontend/ir/patch.h>

namespace Shader::IR {

bool IsGeneric(Patch patch) noexcept {
    return patch >= Patch::Component0 && patch <= Patch::Component119;
}

u32 GenericPatchIndex(Patch patch) {
    if (!IsGeneric(patch)) {
        throw InvalidArgument("Patch {} is not generic", patch);
    }
    return (static_cast<u32>(patch) - static_cast<u32>(Patch::Component0)) / 4;
}

u32 GenericPatchElement(Patch patch) {
    if (!IsGeneric(patch)) {
        throw InvalidArgument("Patch {} is not generic", patch);
    }
    return (static_cast<u32>(patch) - static_cast<u32>(Patch::Component0)) % 4;
}

} // namespace Shader::IR
