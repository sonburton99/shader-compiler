// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#include <string_view>

#include "emit_context.h"
#include "emit_glsl_instructions.h"
#include <shader_compiler/exception.h>

namespace Shader::Backend::GLSL {

void EmitJoin(EmitContext&) {
    throw NotImplementedException("Join shouldn't be emitted");
}

void EmitDemoteToHelperInvocation(EmitContext& ctx) {
    ctx.Add("discard;");
}

} // namespace Shader::Backend::GLSL
