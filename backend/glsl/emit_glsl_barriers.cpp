// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#include "shader_recompiler/backend/glsl/emit_context.h"
#include "shader_recompiler/backend/glsl/emit_glsl_instructions.h"
#include "shader_recompiler/frontend/ir/value.h"

namespace Shader::Backend::GLSL {
void EmitBarrier(EmitContext& ctx) {
    ctx.Add("barrier();");
}

void EmitWorkgroupMemoryBarrier(EmitContext& ctx) {
    ctx.Add("groupMemoryBarrier();");
}

void EmitDeviceMemoryBarrier(EmitContext& ctx) {
    ctx.Add("memoryBarrier();");
}
} // namespace Shader::Backend::GLSL
