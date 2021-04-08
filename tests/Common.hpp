//
// Created by Kirill Delimbetov - github.com/delimbetov - on 07.04.2021.
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <array>
#include <optional>
#include <vector>
#include <gtest/gtest.h>
#include <rune_vm/RuneVm.hpp>

constexpr auto g_wasmBackends = std::array{rune_vm::WasmBackend::Wasm3};
constexpr rune_vm::TThreadCount g_threadCounts[] = {1, 2, 4, 8, 16, 32};
const auto g_testDataDirectory = std::string(TEST_DATA_DIRECTORY);

template<rune_vm::capabilities::Capability capability>
struct CommonTestDelegate : public rune_vm::capabilities::IDelegate {
    CommonTestDelegate(std::vector<uint8_t> input)
        : m_input(std::move(input)) {}

private:
    // rune_vm::capabilities::IDelegate
    [[nodiscard]] std::unordered_set<rune_vm::capabilities::Capability>
    getSupportedCapabilities() const noexcept final {
        return {capability};
    }

    [[nodiscard]] bool requestCapability(
        const rune_vm::capabilities::Capability requestedCapability,
        const rune_vm::capabilities::TId newCapabilityId) noexcept final {
        [&] { ASSERT_EQ(requestedCapability, capability); }();
        m_allocatedId = newCapabilityId;
        return true;
    }

    [[nodiscard]] bool requestCapabilityParamChange(
        const rune_vm::capabilities::TId capabilityId,
        const rune_vm::capabilities::TKey& key,
        const rune_vm::capabilities::Parameter& parameter) noexcept final {
        [&] { ASSERT_EQ(*m_allocatedId, capabilityId); }();
        return true;
    }

    [[nodiscard]] bool requestRuneInputFromCapability(
        const rune_vm::DataView<uint8_t> buffer,
        const rune_vm::capabilities::TId capabilityId) noexcept final {
        [&] { ASSERT_EQ(*m_allocatedId, capabilityId); }();
        [&] { ASSERT_EQ(m_input.size(), buffer.m_size); }();
        std::memcpy(buffer.m_data, m_input.data(), m_input.size());
        return true;
    }

    //
    std::vector<uint8_t> m_input;
    std::optional<rune_vm::capabilities::TId> m_allocatedId;
};