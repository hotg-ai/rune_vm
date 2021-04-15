//
// Created by Kirill Delimbetov - github.com/delimbetov - on 03.04.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <string_view>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <Common.hpp>
#include <OutputManager.hpp>

namespace {
    using namespace rune_vm;
    using namespace rune_vm_internal;

    void parse(LoggingModule& log, const nlohmann::json& json, Result& result) {
        switch(json.type()) {
            case nlohmann::json::value_t::array: {
                auto subresult = std::make_shared<Result>(json.size());

                for(const auto& elem: json) {
                    parse(log, elem, *subresult);
                }

                result.add(std::move(subresult));
                break;
            }
            case nlohmann::json::value_t::string:
                result.add(json.get<std::string>());
                break;
            case nlohmann::json::value_t::number_integer:
                result.add(json.get<int32_t>());
                break;
            case nlohmann::json::value_t::number_unsigned:
                result.add(json.get<uint32_t>());
                break;
            case nlohmann::json::value_t::number_float:
                result.add(json.get<float>());
                break;
            default:
                log.log(Severity::Error, fmt::format("Can't parse json: unexpected type: {}", json.type()));
                CHECK_THROW(false);
        }
    }

    std::shared_ptr<Result> parse(LoggingModule& log, const nlohmann::json& json) {
        auto sizeHint = 1;

        switch(json.type()) {
            case nlohmann::json::value_t::array: {
                sizeHint = json.size();
                break;
            }
            default:
                break;
        }

        auto result = std::make_shared<Result>(sizeHint);

        parse(log, json, *result);

        return result;
    }
}

namespace rune_vm_internal {

    OutputManager::OutputManager(const rune_vm::ILogger::CPtr& logger)
        : m_log(logger, "OutputManager")
        , m_idCounter(0) {
        m_log.log(Severity::Debug, "OutputManager()");
    }

    std::optional<TOutputId> OutputManager::lastSavedId() const noexcept {
        return m_lastSavedId;
    }

    // TODO: pass output type allocation request to the client
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
            m_log.log(
                Severity::Debug,
                fmt::format(
                    "saveOutput: output id={} output={}",
                    outputId,
                    std::string_view(reinterpret_cast<const char*>(buffer.m_data), buffer.m_size)));
            const auto json = nlohmann::json::parse(buffer);
            auto result = parse(m_log, json);

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

    std::optional<Result::Ptr> OutputManager::consumeOutput(const TOutputId outputId) noexcept {
        const auto [iter, found] = find(m_results, outputId);
        if(!found) {
            m_log.log(Severity::Error, fmt::format("Failed to find output id={}", outputId));
            return std::nullopt;
        }

        const auto output = std::move(iter->second);

        m_results.erase(iter);

        if(m_lastSavedId && outputId == *m_lastSavedId)
            m_lastSavedId.reset();

        return output;
    }
}