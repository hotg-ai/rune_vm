//
// Created by Kirill Delimbetov - github.com/delimbetov - on 14.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <fmt/format.h>
#include <rune_vm/RuneVm.hpp>
#include <wasm_backends/Backends.hpp>
#include <inference/Inference.hpp>
#include <Common.hpp>

namespace rune_vm {
    template<typename T>
    VirtualInterface<T>::~VirtualInterface() = default;

    [[nodiscard]] IEngine::Ptr createEngine(
        const ILogger::CPtr& logger,
        const WasmBackend backend,
        const TThreadCount inferenceThreadCount) {
        CHECK_THROW(logger);
        using namespace rune_vm_internal;
        const auto inferenceOptions = inference::IRuntime::Options{inferenceThreadCount};
        const auto inferenceRuntime = rune_vm_internal::inference::createRuntime(
            logger,
            inference::InferenceBackend::TfLite,
            inferenceOptions);
        switch(backend) {
            case WasmBackend::Wasm3:
                return std::make_shared<Wasm3Engine>(logger, inferenceRuntime);
            default:
                logger->log(
                    Severity::Error,
                    "RuneVm.cpp",
                    fmt::format("Unknown backend was requested: {}", backend));
                CHECK_THROW(false);
        }
    }
}