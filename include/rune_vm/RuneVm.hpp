//
// Created by Kirill Delimbetov - github.com/delimbetov - on 14.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <tuple>
#include <vector>
#include <variant>
#include <rune_vm/Capabilities.hpp>
#include <rune_vm/Log.hpp>
#include <rune_vm/VirtualInterface.hpp>

namespace rune_vm {
    enum class Backend: uint8_t {
        Wasm3
    };

    struct IResult : VirtualInterface<IResult> {
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

    struct IRuneResultObserver : VirtualInterface<IRuneResultObserver> {
        virtual void update(IResult* result) const noexcept = 0;
    };

    struct IRune : VirtualInterface<IRune> {
        [[nodiscard]] virtual capabilities::IContext::Ptr getCapabilitiesContext() const noexcept = 0;

        // must be set for rune to be callable
        virtual void attachObserver(IRuneResultObserver::Ptr observer) = 0;
        virtual void detachObserver(IRuneResultObserver::Ptr observer) = 0;

        // calls without waiting for observer call
        virtual void call() = 0;
        // calls and waits for the observer to be called
        virtual void callWaitResult(const std::chrono::microseconds) = 0;
    };

    struct IRuntime : VirtualInterface<IRuntime> {
        // accepts wasm
        // loads manifest
        // returns callable rune
        //
        // delegates - pass delegates, each providing some subset of capabilities.
        // Subsets should not intersect, if that happens, delegate for capability with multiple delegates is chosen randomly
        [[nodiscard]] virtual IRune::Ptr loadRune(
            const std::vector<capabilities::IDelegate::Ptr>& delegates,
            const DataView<const uint8_t> data) = 0;
        [[nodiscard]] virtual IRune::Ptr loadRune(
            const std::vector<capabilities::IDelegate::Ptr>& delegates,
            const std::string_view fileName) = 0;

        //
        [[nodiscard]] virtual std::vector<capabilities::Capability>
            getCapabilitiesWithDefaultDelegates() const noexcept = 0;
    };

    struct IEngine : VirtualInterface<IEngine> {
        // contains wasm environment, logging function
        // if optional is not set for either of args, default values are used
        [[nodiscard]] virtual IRuntime::Ptr createRuntime(
            const std::optional<uint32_t> optStackSizeBytes = std::nullopt,
            const std::optional<uint32_t> optMemoryLimi = std::nullopt) = 0;
    };

    [[nodiscard]] IEngine::Ptr createEngine(
        const Backend backend,
        const ILogger::CPtr& logger);
}
