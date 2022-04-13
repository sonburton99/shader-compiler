// SPDX-License-Identifier: MPL-2.0
// Copyright © 2021 yuzu Emulator Project (https://github.com/yuzu-emu/yuzu/)

#include <string>

#include <fmt/format.h>

#include <shader_compiler/frontend/ir/condition.h>

namespace Shader::IR {

std::string NameOf(Condition condition) {
    std::string ret;
    if (condition.GetFlowTest() != FlowTest::T) {
        ret = fmt::to_string(condition.GetFlowTest());
    }
    const auto [pred, negated]{condition.GetPred()};
    if (!ret.empty()) {
        ret += '&';
    }
    if (negated) {
        ret += '!';
    }
    ret += fmt::to_string(pred);
    return ret;
}

} // namespace Shader::IR
