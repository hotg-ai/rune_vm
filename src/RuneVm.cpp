//
// Created by Kirill Delimbetov - github.com/delimbetov - on 14.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <type_traits>
#include <fmt/format.h>
#include <rune_vm/RuneVm.hpp>
#include <backends/Backends.hpp>
#include <Common.hpp>

namespace rune_vm {
    [[nodiscard]] IEngine::Ptr createEngine(const Backend backend, const ILogger::CPtr& logger) {
        CHECK_THROW(logger);
        switch(backend) {
            case Backend::Wasm3:
                return std::make_shared<rune_vm_internal::Wasm3Engine>(logger);
            default:
                logger->log(
                    Severity::Error,
                    "rune_vm.cpp",
                    fmt::format("Unknown backend was requested: {}", static_cast<std::underlying_type_t<Backend>>(backend)).c_str());
                CHECK_THROW(false);
        }
    }
}