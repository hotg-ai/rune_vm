//
// Created by Kirill Delimbetov - github.com/delimbetov - on 03.04.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <type_traits>
#include <Common.hpp>
#include <Result.hpp>

namespace rune_vm_internal {
    using namespace rune_vm;

    Result::Result(const size_t capacityHint) {
        m_data.reserve(capacityHint);
    }

    void Result::add(const uint32_t data) {
        addInternal(data);
    }

    void Result::add(const int32_t data) {
        addInternal(data);
    }

    void Result::add(const float data) {
        addInternal(data);
    }

    void Result::add(const std::string_view data) {
        CHECK_THROW(data.data());
        addInternal(std::string(data.begin(), data.end()));
    }

    void Result::add(const rune_vm::DataView<const uint8_t> data) {
        CHECK_THROW(data.m_data);
        addInternal(std::vector<uint8_t>(data.begin(), data.end()));
    }

    void Result::add(Result::Ptr&& data) {
        CHECK_THROW(data);
        addInternal(std::move(data));
    }

    template<typename T>
    void Result::addInternal(T&& data) {
        m_data.push_back(std::forward<T>(data));
    }

    // IResult
    IResult::TVariant Result::getAt(const uint32_t idx) const {
        CHECK_THROW(idx < count());
        return std::visit(
            [](auto&& arg) -> IResult::TVariant {
                using TArg = std::decay_t<decltype(arg)>;

                if constexpr(
                    std::is_same_v<uint32_t, TArg> ||
                    std::is_same_v<int32_t, TArg> ||
                    std::is_same_v<float, TArg> ||
                    std::is_same_v<Result::Ptr, TArg>)
                    return arg;
                else if constexpr(std::is_same_v<std::string, TArg>)
                    return std::string_view(arg);
                else if constexpr(std::is_same_v<std::vector<uint8_t>, TArg>)
                    return DataView<const uint8_t>(arg.data(), arg.size());
                else
                    static_assert([](auto) { return false; }(arg));
            },
            m_data[idx]);
    }

    IResult::Type Result::typeAt(const uint32_t idx) const {
        CHECK_THROW(idx < count());
        return std::visit(
            [](auto&& arg) {
                using TArg = std::decay_t<decltype(arg)>;

                if constexpr(std::is_same_v<uint32_t, TArg>)
                    return Type::Uint32;
                else if constexpr(std::is_same_v<int32_t, TArg>)
                    return Type::Int32;
                else if constexpr(std::is_same_v<float, TArg>)
                    return Type::Float;
                else if constexpr(std::is_same_v<std::string, TArg>)
                    return Type::String;
                else if constexpr(std::is_same_v<std::vector<uint8_t>, TArg>)
                    return Type::ByteBuffer;
                else if constexpr(std::is_same_v<Result::Ptr, TArg>)
                    return Type::IResult;
                else
                    static_assert([](auto&&) { return false; }(arg));
            },
            m_data[idx]);
    }

    uint32_t Result::count() const noexcept {
        return m_data.size();
    }
}