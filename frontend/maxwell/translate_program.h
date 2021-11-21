// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#pragma once

#include <shader_compiler/environment.h>
#include <shader_compiler/frontend/ir/basic_block.h>
#include <shader_compiler/frontend/ir/program.h>
#include "control_flow.h"
#include <shader_compiler/host_translate_info.h>
#include <shader_compiler/object_pool.h>

namespace Shader::Maxwell {

[[nodiscard]] IR::Program TranslateProgram(ObjectPool<IR::Inst>& inst_pool,
                                           ObjectPool<IR::Block>& block_pool, Environment& env,
                                           Flow::CFG& cfg, const HostTranslateInfo& host_info);

[[nodiscard]] IR::Program MergeDualVertexPrograms(IR::Program& vertex_a, IR::Program& vertex_b,
                                                  Environment& env_vertex_b);

} // namespace Shader::Maxwell
