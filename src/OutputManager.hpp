//
// Created by Kirill Delimbetov - github.com/delimbetov - on 03.04.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <rune_vm/Log.hpp>
#include <Result.hpp>

namespace rune_vm_internal {
    enum class OutputType: uint8_t {
        Serial,
        Ble,
        Pin,
        Wifi
    };

    using TOutputId = uint32_t;

    class OutputManager {
    public:
        OutputManager(const rune_vm::ILogger::CPtr& logger);

        std::optional<TOutputId> lastSavedId() const noexcept;
        std::optional<TOutputId> requestOutput(const OutputType outputType) noexcept;
        bool saveOutput(const TOutputId outputId, const rune_vm::DataView<const uint8_t> buffer) noexcept;

        // erases result from internal data structure
        std::optional<Result::Ptr> consumeOutput(const TOutputId outputId) noexcept;

    private:
        rune_vm::LoggingModule m_log;
        std::unordered_map<TOutputId, Result::Ptr> m_results;
        TOutputId m_idCounter;
        std::optional<TOutputId> m_lastSavedId;
    };
}


