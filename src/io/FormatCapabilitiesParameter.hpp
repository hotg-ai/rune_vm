//
// Created by Kirill Delimbetov on 30.03.2021.
//

#pragma once

#include <fmt/format.h>
#include <rune_vm/Capabilities.hpp>

template <>
struct fmt::formatter<rune_vm::capabilities::Parameter> : fmt::formatter<std::string> {
    template <typename FormatContext>
    auto format(const rune_vm::capabilities::Parameter& param, FormatContext& ctx) {
        return std::visit(
            [&ctx](const auto& parameter) {
                return format_to(ctx.out(), "{}", parameter);
            },
            param.m_data);

    }
};


