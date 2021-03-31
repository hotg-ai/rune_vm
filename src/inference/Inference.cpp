//
// Created by Kirill Delimbetov - github.com/delimbetov - on 31.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <fmt/format.h>
#include <Common.hpp>
#include <inference/Inference.hpp>
#include <inference/tflite/TfLiteRuntime.hpp>

namespace rune_vm_internal::inference {
    using namespace rune_vm;

    IRuntime::Ptr createRuntime(
        const ILogger::CPtr& logger,
        const InferenceBackend backend,
        const IRuntime::Options& options) {
        CHECK_THROW(logger);
        switch(backend) {
            case InferenceBackend::TfLite:
                return std::make_shared<TfLiteRuntime>(logger, options);
            default:
                logger->log(
                    Severity::Error,
                    "Inference.cpp",
                    fmt::format("Unknown backend was requested: {}", backend));
                CHECK_THROW(false);
        }
    }
}