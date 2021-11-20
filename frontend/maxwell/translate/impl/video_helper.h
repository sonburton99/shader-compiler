// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#pragma once

#include <common/common_types.h>
#include "impl.h"

namespace Shader::Maxwell {
enum class VideoWidth : u64 {
    Byte,
    Unknown,
    Short,
    Word,
};

[[nodiscard]] IR::U32 ExtractVideoOperandValue(IR::IREmitter& ir, const IR::U32& value,
                                               VideoWidth width, u32 selector, bool is_signed);

[[nodiscard]] VideoWidth GetVideoSourceWidth(VideoWidth width, bool is_immediate);

} // namespace Shader::Maxwell
