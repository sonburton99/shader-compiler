// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#include <shader_compiler/backend/glsl/emit_glsl_instructions.h>
#include <shader_compiler/backend/glsl/glsl_emit_context.h>
#include <shader_compiler/exception.h>

namespace Shader::Backend::GLSL {

void EmitJoin(EmitContext&) {
    throw NotImplementedException("Join shouldn't be emitted");
}

void EmitDemoteToHelperInvocation(EmitContext& ctx) {
    ctx.Add("discard;");
}

} // namespace Shader::Backend::GLSL
