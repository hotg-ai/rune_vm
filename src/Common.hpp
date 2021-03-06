//
// Created by Kirill Delimbetov - github.com/delimbetov - on 20.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <stdexcept>
#include <tuple>

#define CHECK_THROW(cond)                                                        \
    do {                                                                         \
        if (!(cond)) {                                                           \
            throw std::runtime_error("Check failed: " #cond " file: " __FILE__); \
        }                                                                        \
    } while (0)

namespace rune_vm_internal {
    namespace constants {
        // that's a bit generous, but it's ok as long as used for rare allocs
        constexpr auto g_cacheLineFriendlyAlignment = 128u;
    }

    template<typename TMap, typename TKey = typename TMap::key_type>
    auto find(TMap& map, const TKey& key) noexcept {
        const auto iter = map.find(key);

        return std::make_pair(iter, iter != map.end());
    }
}
