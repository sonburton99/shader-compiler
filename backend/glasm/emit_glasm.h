// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#pragma once

#include <string>

#include <backend/bindings.h>
#include <frontend/ir/program.h>
#include <profile.h>
#include <runtime_info.h>

namespace Shader::Backend::GLASM {

constexpr u32 PROGRAM_LOCAL_PARAMETER_STORAGE_BUFFER_BASE = 1;

[[nodiscard]] std::string EmitGLASM(const Profile& profile, const RuntimeInfo& runtime_info,
                                    IR::Program& program, Bindings& bindings);

[[nodiscard]] inline std::string EmitGLASM(const Profile& profile, const RuntimeInfo& runtime_info,
                                           IR::Program& program) {
    Bindings binding;
    return EmitGLASM(profile, runtime_info, program, binding);
}

} // namespace Shader::Backend::GLASM
