//
// Created by Kirill Delimbetov - github.com/delimbetov - on 06.04.2021.
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <iostream>
#include "TestLogger.hpp"

const std::vector<TestLogger::Record>& TestLogger::records() const noexcept {
    return m_records;
}

void TestLogger::log(
    const rune_vm::Severity severity,
    const std::string_view module,
    const std::string_view message) const noexcept {
//    std::cout << "[TEST_LOGGER] Severity=" << static_cast<int>(severity) << " Module=" << module << " Message=" << message << std::endl;
    m_records.push_back(Record{severity, std::string(module), std::string(message)});
}