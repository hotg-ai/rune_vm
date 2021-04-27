//
// Created by Kirill Delimbetov - github.com/delimbetov - on 03.04.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <string_view>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <Common.hpp>
#include <OutputManager.hpp>

namespace rune_vm_internal {
    using namespace rune_vm;

    OutputManager::OutputManager(const rune_vm::ILogger::CPtr& logger)
        : m_log(logger, "OutputManager")
        , m_idCounter(0) {
        m_log.log(Severity::Debug, "OutputManager()");
    }

    std::optional<TOutputId> OutputManager::lastSavedId() const noexcept {
        return m_lastSavedId;
    }

    // TODO: pass output type allocation request to the client
    // TODO: once its done, remove lastSavedId
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

        // clean cache in case there's some error
        constexpr auto cacheSizeLimit = 10'000;
        if(m_results.size() > cacheSizeLimit) {
            m_log.log(
                Severity::Warning,
                fmt::format("requestOutput: size of cache={} exceeds limit={}. Reset", m_results.size(), cacheSizeLimit));
            m_results.clear();
        }

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
            fmt::format("New output of type={} allocated with id={}. Cache size={}", outputType, outputId, m_results.size()));

        return outputId;
    }

    bool OutputManager::saveOutput(const TOutputId outputId, const rune_vm::DataView<const uint8_t> buffer) noexcept {
        m_log.log(
            Severity::Debug,
            fmt::format("saveOutput: output id={} data size={} is being saved", outputId, buffer.m_size));
        auto [iter, found] = find(m_results, outputId);
        if(!found) {
            m_log.log(Severity::Error, fmt::format("saveOutput: output id={} was not requested", outputId));
            return false;
        }

        // atm we only work with Serial and it's guaranteed to be json
        try {
            const auto bufferStringView = std::string_view(
                reinterpret_cast<const char*>(buffer.m_data),
                buffer.m_size);
            m_log.log(
                Severity::Debug,
                fmt::format("saveOutput: output id={} output={}", outputId, bufferStringView));
            auto result = std::make_shared<JsonResult>(bufferStringView);

            // save
            iter->second = std::move(result);
            m_lastSavedId = iter->first;
        } catch(const std::exception& e) {
            m_log.log(
                Severity::Error,
                fmt::format(
                    "Failed to deserialize output: {} to json",
                    std::string_view(reinterpret_cast<const char*>(buffer.m_data), buffer.m_size)));
            return false;
        }

        return true;
    }

    std::optional<rune_vm::IResult::Ptr> OutputManager::consumeOutput(const TOutputId outputId) noexcept {
        const auto [iter, found] = find(m_results, outputId);
        if(!found) {
            m_log.log(Severity::Error, fmt::format("Failed to find output id={}", outputId));
            return std::nullopt;
        }

        const auto output = std::move(iter->second);

        iter->second.reset();

        return output;
    }
}
