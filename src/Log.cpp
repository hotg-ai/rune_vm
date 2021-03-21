//
// Created by Kirill Delimbetov - github.com/delimbetov - on 21.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <rune_vm/Log.hpp>
#include <Common.hpp>

namespace rune_vm {
    LoggingModule::LoggingModule(const ILogger::CPtr& logger, const std::string& module)
        : m_logger(logger)
        , m_module(module) {
        CHECK_THROW(logger);
    }

    void LoggingModule::log(const Severity severity, const std::string_view message) const noexcept {
        m_logger->log(severity, m_module, message)
    }

    const ILogger::CPtr& LoggingModule::logger() const noexcept {
        return m_logger;
    }
}