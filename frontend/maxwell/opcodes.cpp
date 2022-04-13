// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#include <array>

#include <shader_compiler/exception.h>
#include <shader_compiler/frontend/maxwell/opcodes.h>

namespace Shader::Maxwell {
namespace {
constexpr std::array NAME_TABLE{
#define INST(name, cute, encode) cute,
#include "maxwell.inc"
#undef INST
};
} // Anonymous namespace

const char* NameOf(Opcode opcode) {
    if (static_cast<size_t>(opcode) >= NAME_TABLE.size()) {
        throw InvalidArgument("Invalid opcode with raw value {}", static_cast<int>(opcode));
    }
    return NAME_TABLE[static_cast<size_t>(opcode)];
}

} // namespace Shader::Maxwell
