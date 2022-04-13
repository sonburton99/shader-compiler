// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#include <shader_compiler/backend/glasm/emit_glasm_instructions.h>
#include <shader_compiler/backend/glasm/glasm_emit_context.h>

namespace Shader::Backend::GLASM {

void EmitBarrier(EmitContext& ctx) {
    ctx.Add("BAR;");
}

void EmitWorkgroupMemoryBarrier(EmitContext& ctx) {
    ctx.Add("MEMBAR.CTA;");
}

void EmitDeviceMemoryBarrier(EmitContext& ctx) {
    ctx.Add("MEMBAR;");
}

} // namespace Shader::Backend::GLASM
