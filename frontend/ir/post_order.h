// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#pragma once

#include <shader_compiler/frontend/ir/abstract_syntax_list.h>
#include <shader_compiler/frontend/ir/basic_block.h>

namespace Shader::IR {

BlockList PostOrder(const AbstractSyntaxNode& root);

} // namespace Shader::IR
