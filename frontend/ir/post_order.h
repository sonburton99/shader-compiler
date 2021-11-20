// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#pragma once

#include "abstract_syntax_list.h"
#include "basic_block.h"

namespace Shader::IR {

BlockList PostOrder(const AbstractSyntaxNode& root);

} // namespace Shader::IR
