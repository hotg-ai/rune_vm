//
// Created by Kirill Delimbetov - github.com/delimbetov - on 14.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <variant>
#include <rune_vm/Log.hpp>

namespace rune_vm {
    enum class Backend: uint8_t {
        Wasm3
    };

    template<typename T>
    struct DataView {
        DataView(T* data, uint64_t size)
            : m_data(data)
            , m_size(size) {}

        const T* m_data;
        const uint64_t m_size;
    };

    struct IResult {
        enum class Type: uint8_t {
            Uint32,
            String,
            ByteBuffer,
            IResult
        };

        [[nodiscard]] virtual std::variant<uint32_t, std::string_view, DataView<const uint8_t>, IResult*> getAt(const uint32_t idx) const = 0;
        [[nodiscard]] virtual Type typeAt(const uint32_t idx) const = 0;
        [[nodiscard]] virtual uint32_t count() const noexcept = 0;
    };

    struct IRuneResultObserver {
        using Ptr = std::shared_ptr<IRuneResultObserver>;

        virtual void update(IResult* result) const noexcept = 0;
    };

    struct IRune {
        using Ptr = std::shared_ptr<IRune>;

        // must be set for rune to be callable
        virtual void attachObserver(IRuneResultObserver::Ptr observer) = 0;
        virtual void detachObserver(IRuneResultObserver::Ptr observer) = 0;

        // calls without waiting for observer call
        virtual void call() = 0;
        // calls and waits for the observer to be called
        virtual void callWaitResult(const std::chrono::microseconds) = 0;
    };

    struct IRuntime {
        using Ptr = std::shared_ptr<IRuntime>;

        // accepts wasm
        // loads manifest
        // returns callable rune
        [[nodiscard]] virtual IRune::Ptr loadRune(const DataView<const uint8_t> data) = 0;
        [[nodiscard]] virtual IRune::Ptr loadRune(const std::string_view fileName) = 0;
    };

    struct IEngine {
        using Ptr = std::shared_ptr<IEngine>;

        // contains wasm environment, logging function
        // if optional is not set for either of args, default values are used
        [[nodiscard]] virtual IRuntime::Ptr createRuntime(
            const std::optional<uint32_t> optStackSizeBytes,
            const std::optional<uint32_t> optMemoryLimit) = 0;
    };

    [[nodiscard]] IEngine::Ptr createEngine(
        const Backend backend,
        const ILogger::CPtr& logger);
}
