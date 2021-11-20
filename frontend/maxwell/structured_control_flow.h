// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#pragma once

#include <environment.h>
#include <frontend/ir/abstract_syntax_list.h>
#include <frontend/ir/basic_block.h>
#include <frontend/ir/value.h>
#include "control_flow.h"
#include <object_pool.h>

namespace Shader {
struct HostTranslateInfo;
namespace Maxwell {

[[nodiscard]] IR::AbstractSyntaxList BuildASL(ObjectPool<IR::Inst>& inst_pool,
                                              ObjectPool<IR::Block>& block_pool, Environment& env,
                                              Flow::CFG& cfg, const HostTranslateInfo& host_info);

} // namespace Maxwell
} // namespace Shader
