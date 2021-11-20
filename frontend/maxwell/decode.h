// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#pragma once

#include <common/common_types.h>
#include "opcodes.h"

namespace Shader::Maxwell {

[[nodiscard]] Opcode Decode(u64 insn);

} // namespace Shader::Maxwell
