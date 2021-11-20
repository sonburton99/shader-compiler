// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#pragma once

#include <string>

#include <backend/bindings.h>
#include <frontend/ir/program.h>
#include <profile.h>
#include <runtime_info.h>

namespace Shader::Backend::GLSL {

[[nodiscard]] std::string EmitGLSL(const Profile& profile, const RuntimeInfo& runtime_info,
                                   IR::Program& program, Bindings& bindings);

[[nodiscard]] inline std::string EmitGLSL(const Profile& profile, IR::Program& program) {
    Bindings binding;
    return EmitGLSL(profile, {}, program, binding);
}

} // namespace Shader::Backend::GLSL
