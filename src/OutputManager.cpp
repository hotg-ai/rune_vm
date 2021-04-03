//
// Created by Kirill Delimbetov - github.com/delimbetov - on 03.04.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <fmt/format.h>
#include <OutputManager.hpp>

namespace rune_vm_internal {
    using namespace rune_vm;

    OutputManager::OutputManager(const rune_vm::ILogger::CPtr& logger)
        : m_log(logger, "OutputManager")
        , m_idCounter(0) {
        m_log.log(Severity::Debug, "OutputManager()");
    }

    std::optional<TOutputId> OutputManager::requestOutput(const OutputType outputType) noexcept {
        if(outputType != OutputType::Serial) {
            m_log.log(
                Severity::Error,
                fmt::format(
                    "requestOutput: only serial output type is supported atm, yet {} is requested",
                    outputType));
            return std::nullopt;
        }

        const auto outputId = m_idCounter;

        // cache
        m_results[outputId] = nullptr;

        ++m_idCounter;

        if(m_idCounter == std::numeric_limits<decltype(m_idCounter)>::max()) {
            m_log.log(
                Severity::Warning,
                fmt::format("Output id counter has hit max value={}. Next allocation will overflow it", m_idCounter));
        }

        m_log.log(
            Severity::Info,
            fmt::format("New output of type={} allocated with id={}", outputType, outputId));
        return outputId;
    }

    bool OutputManager::saveOutput(const TOutputId outputId, const rune_vm::DataView<const uint8_t> buffer) noexcept {
#error impl
    }

    Result::Ptr OutputManager::consumeOutput(const TOutputId outputId) {
#error impl
    }
}