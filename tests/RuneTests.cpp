//
// Created by Kirill Delimbetov - github.com/delimbetov - on 06.04.2021.
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <cmath>
#include <random>
#include <gtest/gtest.h>
#include <rune_vm/RuneVm.hpp>
#include "Common.hpp"
#include "TestLogger.hpp"

using namespace rune_vm;

struct Delegate : public capabilities::IDelegate {
    Delegate(const float input)
        : m_input(input) {}

private:
    // rune_vm::capabilities::IDelegate
    [[nodiscard]] std::unordered_set<rune_vm::capabilities::Capability>
        getSupportedCapabilities() const noexcept final {
        return {capabilities::Capability::Rand};
    }

    [[nodiscard]] bool requestCapability(
        const rune_vm::capabilities::Capability capability,
        const rune_vm::capabilities::TId newCapabilityId) noexcept final {
        return true;
    }

    [[nodiscard]] bool requestCapabilityParamChange(
        const rune_vm::capabilities::TId capabilityId,
        const rune_vm::capabilities::TKey& key,
        const rune_vm::capabilities::Parameter& parameter) noexcept final {
        return true;
    }

    [[nodiscard]] bool requestRuneInputFromCapability(
        const rune_vm::DataView<uint8_t> buffer,
        const rune_vm::capabilities::TId capabilityId) noexcept final {
        std::memcpy(buffer.m_data, &m_input, sizeof(m_input));
        return true;
    }

    //
    float m_input;
};
struct SineRuneTest
    : public testing::TestWithParam<
        std::tuple<WasmBackend, TThreadCount, std::optional<uint32_t>, std::optional<uint32_t>>> {

    auto loadAndRun(float& output, const std::vector<capabilities::IDelegate::Ptr>& delegates) {
        const auto [backend, threadCount, optStackSizeBytes, optMemoryLimit] = GetParam();
        const auto logger = std::make_shared<const TestLogger>();
        auto engine = createEngine(logger, backend, threadCount);
        auto runtime = engine->createRuntime(optStackSizeBytes, optMemoryLimit);
        auto rune = runtime->loadRune(delegates, g_testDataDirectory + "/sine/sine.rune");
        ASSERT_TRUE(rune);

        // sine rune outputs [float]
        // assert top-level result is array
        auto result = rune->call();
        ASSERT_TRUE(result);
        ASSERT_EQ(result->count(), 1);
        ASSERT_EQ(result->typeAt(0), rune_vm::IResult::Type::IResult);

        // assert it was array with 1 float
        const auto firstElement = result->getAt(0);
        ASSERT_TRUE(std::holds_alternative<rune_vm::IResult::Ptr>(firstElement));
        const auto& elementResult = std::get<rune_vm::IResult::Ptr>(firstElement);
        ASSERT_EQ(elementResult->count(), 1);
        ASSERT_EQ(elementResult->typeAt(0), rune_vm::IResult::Type::Float);
        const auto valueVariant = elementResult->getAt(0);
        ASSERT_TRUE(std::holds_alternative<float>(valueVariant));
        const auto& value = std::get<float>(valueVariant);
        ASSERT_LE(value, 1.f);
        ASSERT_GE(value, -1.f);

        output = value;
    }

    // setup
    void SetUp() override {
        std::random_device rd;
        m_engine = std::mt19937(rd());
        m_distrib = std::uniform_real_distribution<>(0.f, 2 * M_PI);
    }

    std::mt19937 m_engine;
    std::uniform_real_distribution<> m_distrib;
};

TEST_P(SineRuneTest, DefaultRandDelegate) {
    auto output = 100.f;
    loadAndRun(output, {});
}

TEST_P(SineRuneTest, CustomRandDelegate) {
    const auto input = m_distrib(m_engine);
    const auto delegate = std::make_shared<Delegate>(input);
    auto output = 100.f;

    loadAndRun(output, {delegate});
    // sine rune is inexact
    ASSERT_NEAR(output, std::sin(input), 0.1f);
}

TEST_P(SineRuneTest, CustomRandDelegateKnownValue) {
    const auto input = 0.8f;
    const auto delegate = std::make_shared<Delegate>(input);
    auto output = 100.f;

    loadAndRun(output, {delegate});
    // sine rune is inexact
    ASSERT_NEAR(output, 6.972786e-1, 0.0001f);
}

INSTANTIATE_TEST_SUITE_P(
    CorrectArguments,
    SineRuneTest,
    testing::Combine(
        testing::ValuesIn(g_wasmBackends),
        testing::ValuesIn(g_threadCounts),
        testing::Values(std::nullopt, 1 << 15, 1 << 20, uint32_t(-1)),
        testing::Values(std::nullopt, 1 << 25, uint32_t(-1))));
