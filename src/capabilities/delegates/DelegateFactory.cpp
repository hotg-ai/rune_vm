//
// Created by Kirill Delimbetov - github.com/delimbetov - on 30.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <unordered_map>
#include <tuple>
#include <Common.hpp>
#include <capabilities/CapabilitiesDelegatesManager.hpp>
#include <capabilities/delegates/RandomCapabilityDelegate.hpp>

namespace {
    // Props to https://stackoverflow.com/users/1938163/marco-a
    template<unsigned... Is> struct seq{};
    template<unsigned N, unsigned... Is>
    struct gen_seq : gen_seq<N-1, N-1, Is...>{};
    template<unsigned... Is>
    struct gen_seq<0, Is...> : seq<Is...>{};

    template<unsigned N1, unsigned... I1, unsigned N2, unsigned... I2>
    // Expansion pack
    constexpr std::array<int, N1+N2> concat(const std::array<int, N1>& a1, const std::array<int, N2>& a2, seq<I1...>, seq<I2...>){
        return { a1[I1]..., a2[I2]... };
    }

    template<unsigned N1, unsigned N2>
    // Initializer for the recursion
    constexpr std::array<int, N1+N2> concat(const std::array<int, N1>& a1, const std::array<int, N2>& a2){
        return concat(a1, a2, gen_seq<N1>{}, gen_seq<N2>{});
    }
    //

    using namespace rune_vm_internal;

    template<size_t currTupleElement, typename TTuple>
    constexpr auto concatCapabilities(const TTuple& tuple) noexcept {
        static_assert(std::tuple_size_v<TTuple> > currTupleElement);
        if constexpr(currTupleElement == std::tuple_size_v<TTuple> - 1)
            return std::get<currTupleElement>(tuple);
        else
            return concat(std::get<currTupleElement>(tuple), concatCapabilities<currTupleElement + 1>(tuple));
    }

    template<typename ...TDelegates>
    constexpr auto flattenArrays() noexcept {
        constexpr auto tuple = std::make_tuple(TDelegates::supportedCapabilities()...);
        return concatCapabilities<0>(tuple);
    }

    constexpr auto g_allSupportedDefaultCapabilities = flattenArrays<
            // list of default delegates
            RandomCapabilityDelegate
            //
            >();

    using TDelegateFactory = std::function<rune_vm::capabilities::IDelegate::Ptr(const rune_vm::ILogger::CPtr&)>;
    using TMap = std::unordered_map<rune_vm::capabilities::Capability, TDelegateFactory>;

    template<typename TDelegate>
    void registerDelegate(TMap& map) {
        std::transform(
            TDelegate::supportedCapabilities().begin(),
            TDelegate::supportedCapabilities().end(),
            std::inserter(map, map.begin()),
            [](const auto capability) {
                return std::make_pair(
                    capability,
                    TDelegateFactory([](const rune_vm::ILogger::CPtr& logger) { return std::make_shared<TDelegate>(logger); }));
            });
    }

    template<typename ...TDelegates>
    void registerDelegates(TMap& map) {
        (registerDelegate<TDelegates>(map),...);
    }

    const auto g_defaultDelegateFactories = [] {
        // TODO: it should be multimap
        auto map = std::unordered_map<rune_vm::capabilities::Capability, TDelegateFactory>();

        registerDelegates<RandomCapabilityDelegate>(map);

        return map;
    }();
}

namespace rune_vm_internal {
    using namespace rune_vm;

    std::vector<capabilities::Capability> getAllSupportedByDefaultCapabilities() {
        return {g_allSupportedDefaultCapabilities.begin(), g_allSupportedDefaultCapabilities.end()};
    }

    std::optional<capabilities::IDelegate::Ptr> getDefaultDelegateImplementingCapability(
        const rune_vm::ILogger::CPtr& logger,
        const capabilities::Capability capability) {
        const auto [iter, found] = find(g_defaultDelegateFactories, capability);
        if(!found)
            return std::nullopt;

        return iter->second(logger);
    }
}