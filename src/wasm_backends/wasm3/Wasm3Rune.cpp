//
// Created by Kirill Delimbetov - github.com/delimbetov - on 14.03.2021
// Copyright (c) HAMMER OF THE GODS INC. - hotg.ai
//

#include <array>
#include <type_traits>
#include <fmt/format.h>
#include <m3_api_defs.h>
#include <m3_api_wasi.h>
#include <m3_env.h>
#include <wasm3.h>
#include <Common.hpp>
#include <HostFunctions.hpp>
#include <wasm_backends/wasm3/Wasm3Common.hpp>
#include <wasm_backends/wasm3/Wasm3Rune.hpp>

using namespace rune_vm_internal;

namespace {
    namespace wasm3_interop {
        // copied from wasm3 mostly
        using TStackType = uint64_t*;
        using TMemType = void*;

        void checkMemoryThrow(void* memory, uint64_t size, IM3Runtime runtime, TMemType _mem) {
            const auto res = [memory, size, runtime, _mem] {
                m3ApiCheckMem(memory, size);
                return m3Err_trapOutOfBoundsMemoryAccess + 1; }();
            CHECK_THROW(res != m3Err_trapOutOfBoundsMemoryAccess);
        }

        template<typename T>
        void checkMemoryThrow(const TStackType stack, IM3Runtime runtime, TMemType _mem) {
            checkMemoryThrow(stack, sizeof(T), runtime, _mem);
        }

        template<typename T>
        void checkMemoryThrow(const TStackType stack, rune_vm::DataView<T> dest, IM3Runtime runtime, TMemType _mem) {
            checkMemoryThrow(stack, sizeof(dest.m_data) + sizeof(dest.m_size), runtime, _mem);
        }

        template<typename T, std::enable_if_t<std::is_enum_v<T>, int> = 0>
        void arg_from_stack(T &dest, TStackType &psp, IM3Runtime runtime, TMemType _mem) {
            checkMemoryThrow<rune_interop::TIntType>(psp, runtime, _mem);
            dest = rune_interop::fromInt<T>(*(rune_interop::TIntType *) (psp));
            psp++;
        }

        template<typename T, std::enable_if_t<!std::is_enum_v<T>, int> = 0>
        void arg_from_stack(T &dest, TStackType &psp, IM3Runtime runtime, TMemType _mem) {
            checkMemoryThrow<T>(psp, runtime, _mem);
            dest = *(T *) (psp);
            psp++;
        }

        template<typename T, typename TSize>
        void arg_from_stack(rune_vm::DataView<T, TSize> dest, TStackType &psp, IM3Runtime runtime, TMemType _mem) {
            checkMemoryThrow(psp, dest, runtime, _mem);
            dest.m_data = (T*) m3ApiOffsetToPtr(* ((u32 *) (psp++)));
            dest.m_size = *(TSize *) (psp++);
        }

        template<typename T>
        void arg_from_stack(T* &dest, TStackType &psp, IM3Runtime runtime, TMemType _mem) {
            checkMemoryThrow<T*>(psp, runtime, _mem);
            dest = (void*) m3ApiOffsetToPtr(* ((u32 *) (psp++)));
        };

        template<typename T>
        void arg_from_stack(const T* &dest, TStackType &psp, IM3Runtime runtime, TMemType _mem) {
            checkMemoryThrow<T*>(psp, runtime, _mem);
            dest = (void*) m3ApiOffsetToPtr(* ((u32 *) (psp++)));
        };

        template <typename ...Args>
        static void get_args_from_stack(TStackType &sp, IM3Runtime runtime, TMemType mem, std::tuple<Args...> &tuple) {
            std::apply([&](auto &... item) {
                (arg_from_stack(item, sp, runtime, mem), ...);
            }, tuple);
        }

        template<auto func>
        struct wrap_helper;

        template <typename Ret, typename ...Args, Ret (*Fn)(Args...)>
        struct wrap_helper<Fn> {
            static const void *wrap_fn(IM3Runtime rt, IM3ImportContext _ctx, TStackType sp, TMemType mem) {
                Ret *ret_ptr = (Ret *) (sp);
                std::tuple<Args...> args;
                try {
                    get_args_from_stack(sp, rt, mem, args);
                } catch(...) {
                    // can't properly log it here
                    static_assert(std::is_same_v<Ret, std::underlying_type_t<rune_interop::ReturnCode>>);
                    *ret_ptr = rune_interop::RC_InputError;
                    return m3Err_malformedFunctionSignature;
                }
                Ret r = std::apply(Fn, args);
                *ret_ptr = r;
                return m3Err_none;
            }
        };

        template <typename Ret, typename ...Args, Ret (*Fn)(host_functions::IHostContext*, Args...)>
        struct wrap_helper<Fn> {
            static const void *wrap_fn(IM3Runtime rt, IM3ImportContext _ctx, TStackType sp, TMemType mem) {
                Ret *ret_ptr = (Ret *) (sp);
                auto* context = reinterpret_cast<host_functions::IHostContext*>(_ctx->userdata);
                std::tuple<Args...> args;
                try {
                    CHECK_THROW(context);
                    get_args_from_stack(sp, rt, mem, args);
                } catch(...) {
                    // can't properly log it here
                    static_assert(std::is_same_v<Ret, std::underlying_type_t<rune_interop::ReturnCode>>);
                    *ret_ptr = rune_interop::RC_InputError;
                    return m3Err_malformedFunctionSignature;
                }
                Ret r = std::apply(Fn, std::tuple_cat(std::make_tuple(context), std::move(args)));
                *ret_ptr = r;
                return m3Err_none;
            }
        };

        template <typename ...Args, void (*Fn)(Args...)>
        struct wrap_helper<Fn> {
            static const void *wrap_fn(IM3Runtime rt, IM3ImportContext _ctx, TStackType sp, TMemType mem) {
                std::tuple<Args...> args;
                try {
                    get_args_from_stack(sp, rt, mem, args);
                } catch(...) {
                    return m3Err_malformedFunctionSignature;
                }
                std::apply(Fn, args);
                return m3Err_none;
            }
        };

        template<typename... Arrs>
        constexpr auto concat(const Arrs&... arrs) {
            std::size_t cur = 0;
            std::array<std::common_type_t<typename Arrs::value_type...>, (std::tuple_size_v<Arrs> + ...)> result{};
            ([&](const auto& arr) {
                for (const auto& elem : arr) {
                    result[cur++] = elem;
                }
            }(arrs), ...);

            return result;
        }

        template<char ...c>
        struct m3_sig {
            static constexpr std::array<char, sizeof...(c)> value = {c...};
        };

        template<typename T, typename T2 = void> struct m3_type_to_sig;
        template<typename T> struct m3_type_to_sig<T, std::enable_if_t<std::is_enum_v<T>>> : m3_sig<'i'> {};
        template<> struct m3_type_to_sig<i32> : m3_sig<'i'> {};
        template<> struct m3_type_to_sig<u32> : m3_sig<'i'> {};
        template<> struct m3_type_to_sig<i64> : m3_sig<'I'> {};
        template<> struct m3_type_to_sig<f32> : m3_sig<'f'> {};
        template<> struct m3_type_to_sig<f64> : m3_sig<'F'> {};
        template<> struct m3_type_to_sig<void> : m3_sig<'v'> {};
        template<> struct m3_type_to_sig<void *> : m3_sig<'*'> {};
        template<> struct m3_type_to_sig<const void *> : m3_sig<'*'> {};
        template<typename T> struct m3_type_to_sig<rune_vm::DataView<T>> : m3_sig<'*', 'i'> {};


        template<typename Ret, typename ... Args>
        struct m3_signature {
            constexpr static size_t n_args = sizeof...(Args);
            constexpr static auto value = concat(
                m3_type_to_sig<Ret>::value,
                m3_sig<'('>::value,
                m3_type_to_sig<Args>::value...,
                m3_sig<')'>::value,
                m3_sig<'\0'>::value
            );
        };

        template<typename Ret, typename ...Args>
        constexpr auto getSignature(Ret (*)(Args...)) {
            return m3_signature<Ret, Args...>::value;
        }

        template<typename Ret, typename ...Args>
        constexpr auto getSignature(Ret (*)(host_functions::IHostContext*, Args...)) {
            return m3_signature<Ret, Args...>::value;
        }
    }
}

namespace rune_vm_internal {
    using namespace rune_vm;

    Wasm3Rune::Wasm3Rune(
        const rune_vm::ILogger::CPtr& logger,
        std::shared_ptr<M3Module> module,
        std::shared_ptr<M3Runtime> runtime,
        std::shared_ptr<CapabilitiesDelegatesManager>&& manager)
        : m_log(logger, "Wasm3Rune")
        , m_module(std::move(module))
        , m_runtime(std::move(runtime))
        , m_manager(std::move(manager)) {
        m_log.log(Severity::Debug, "Wasm3Rune()");

        // Link host functions
        using namespace rune_interop::host_function_rune_name;
        link<g_requestCapability>();
        link<g_requestCapabilitySetParam>();
        link<g_requestProviderResponse>();
        link<g_tfmPreloadModel>();
        link<g_tfmModelInvoke>();
        link<g_requestOutput>();
        link<g_consumeOutput>();
        link<g_debug>();

        // Lookup manifest function
        auto manifestFunction = IM3Function();

        checkedCall(
            m_log,
            m_runtime,
            m3_FindFunction,
            &manifestFunction,
            m_runtime.get(),
            rune_interop::rune_function_name::g_manifest);
        CHECK_THROW(manifestFunction);

        // Call manifest function
        checkedCall(
            m_log,
            m_runtime,
            m3_CallArgv,
            manifestFunction,
            0,
            nullptr);
    }

    // IRune
    capabilities::IContext::Ptr Wasm3Rune::getCapabilitiesContext() const noexcept {
        return m_manager;
    }

    void attachObserver(rune_vm::IRuneResultObserver::Ptr observer);
    void detachObserver(rune_vm::IRuneResultObserver::Ptr observer);

    // calls without waiting for observer call
    void call();
    // calls and waits for the observer to be called
    void callWaitResult(const std::chrono::microseconds);

    // Internal
    template<auto functionName>
    void Wasm3Rune::link() {
        constexpr auto function = host_functions::nameToFunctionMap<functionName>();
        constexpr auto signature = wasm3_interop::getSignature(function);

        m_log.log(
            Severity::Info,
            fmt::format("Linking to function name={} signature={}", functionName, signature.data()));
        checkedCall(
            m_log,
            m_runtime,
            m3_LinkRawFunctionEx,
            m_module.get(),
            rune_interop::g_moduleName,
            functionName,
            signature.data(),
            wasm3_interop::wrap_helper<function>::wrap_fn,
            this);
    }
}
