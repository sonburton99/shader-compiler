// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#include <shader_compiler/backend/glsl/emit_glsl_instructions.h>
#include <shader_compiler/backend/glsl/glsl_emit_context.h>

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
