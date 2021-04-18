//
// Created by Kirill Delimbetov - github.com/delimbetov - on 06.04.2021.
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <rune_vm/Log.hpp>
#include <vector>

struct TestLogger : public rune_vm::ILogger {
    struct Record {
        rune_vm::Severity m_severity;
        std::string m_module;
        std::string m_message;
    };

    const std::vector<Record>& records() const noexcept;

    // ILogger
    void log(
        const rune_vm::Severity severity,
        const std::string_view module,
        const std::string_view message) const noexcept final;

private:
    // data
    mutable std::vector<Record> m_records;
};
