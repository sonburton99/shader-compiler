// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#include <shader_compiler/backend/glasm/emit_glasm_instructions.h>
#include <shader_compiler/backend/glasm/glasm_emit_context.h>

namespace Shader::Backend::GLASM {

void EmitJoin(EmitContext&) {
    throw NotImplementedException("Join shouldn't be emitted");
}

void EmitDemoteToHelperInvocation(EmitContext& ctx) {
    ctx.Add("KIL TR.x;");
}

} // namespace Shader::Backend::GLASM
