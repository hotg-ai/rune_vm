//
// Created by Kirill Delimbetov - github.com/delimbetov - on 14.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <memory>
#include <string_view>
#include <string>
#include <rune_vm/VirtualInterface.hpp>

namespace rune_vm {
    enum class Severity: uint8_t {
        Debug,
        Info,
        Warning,
        Error
    };

    // TODO: Add default loggers
    struct ILogger: VirtualInterface<ILogger> {
        virtual void log(
            const Severity severity,
            const std::string_view module,
            const std::string_view message) const noexcept = 0;
    };

    class LoggingModule {
    public:
        LoggingModule(const ILogger::CPtr& logger, const std::string& module);

        void log(const Severity severity, const std::string_view message) const noexcept;

        const ILogger::CPtr& logger() const noexcept;

    private:
        ILogger::CPtr m_logger;
        std::string m_module;
    };
}