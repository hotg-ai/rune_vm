//
// Created by Kirill Delimbetov - github.com/delimbetov - on 03.04.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#pragma once

#include <rune_vm/RuneVm.hpp>

namespace rune_vm_internal {
    class Result : public rune_vm::IResult {
    public:
        using TInternalVariant = std::variant<uint32_t, int32_t, float, std::string, std::vector<uint8_t>, Result::Ptr>;

        Result(const size_t capacityHint);

        void add(const uint32_t data);
        void add(const int32_t data);
        void add(const float data);
        void add(const std::string_view data);
        void add(const rune_vm::DataView<const uint8_t> data);
        void add(Result::Ptr&& data);

    private:
        template<typename T>
        void addInternal(T&&);

        // IResult
        [[nodiscard]] TVariant getAt(const uint32_t idx) const final;
        [[nodiscard]] Type typeAt(const uint32_t idx) const final;
        [[nodiscard]] uint32_t count() const noexcept final;

        // data
        std::vector<TInternalVariant> m_data;
    };
}
