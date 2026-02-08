#pragma once

#include "../replay-context.h"
#include "../replay-shared.h"
#include "proxy-base.h"

#include <tuple>
#include <type_traits>

namespace SlangRecord
{

// =============================================================================
// Core Macros - Fine-grained control for recording API calls
// =============================================================================

// Insert a labeled marker into the replay stream for debugging.
// Usage: SLANG_REPLAY_MARKER("before createSession")
#define SLANG_REPLAY_MARKER(label)          \
    do                                      \
    {                                       \
        ReplayContext::get().marker(label); \
    } while (0)

// Begins a recorded instance method call - locks context, records signature and 'this'
// Uses compiler-specific macro for full signature including type name
#define RECORD_CALL()                  \
    auto& _ctx = ReplayContext::get(); \
    auto _lock = _ctx.lock();          \
    _ctx.beginCall(SLANG_FUNC_SIG, this)

// For static/free functions (no 'this' pointer)
#define RECORD_STATIC_CALL()           \
    auto& _ctx = ReplayContext::get(); \
    auto _lock = _ctx.lock();          \
    _ctx.beginStaticCall(SLANG_FUNC_SIG)

// Record an input parameter
// Note: We cast away const for inputs since recording only reads the value
#define RECORD_INPUT(arg)  \
    _ctx.record(           \
        RecordFlag::Input, \
        const_cast<std::remove_const_t<std::remove_reference_t<decltype(arg)>>&>(arg))

// Record an input array parameter
#define RECORD_INPUT_ARRAY(arr, count) _ctx.recordArray(RecordFlag::Input, arr, count)

// Record informational data (neither input nor output, just for stream documentation)
#define RECORD_INFO(arg)  \
    _ctx.record(          \
        RecordFlag::None, \
        const_cast<std::remove_const_t<std::remove_reference_t<decltype(arg)>>&>(arg))

// Record an output parameter (for non-COM T* outputs, no wrapping)
#define RECORD_OUTPUT(arg) _ctx.record(RecordFlag::Output, *arg)

// Prepare a pointer output parameter (T** style)
// Creates a temporary local variable if the pointer is null
// Usage: PREPARE_POINTER_OUTPUT(outBlob) where outBlob is ISlangBlob**
//        or PREPARE_POINTER_OUTPUT(pathTypeOut) where pathTypeOut is SlangPathType*
#define PREPARE_POINTER_OUTPUT(arg)             \
    std::decay_t<decltype(*arg)> _temp_##arg{}; \
    if (!arg)                                   \
    arg = &_temp_##arg

// Record an output parameter (for T** style outputs, dereferences and wraps)
#define RECORD_COM_OUTPUT(arg) _ctx.record(RecordFlag::Output, *arg)

// Record a blob output parameter (ISlangBlob**)
// Serializes blob by content hash to disk. During playback, loads from disk.
// No proxy wrapping is needed for blobs.
// Usage: RECORD_BLOB_OUTPUT(outBlob) where outBlob is ISlangBlob**
#define RECORD_BLOB_OUTPUT(arg) RECORD_COM_OUTPUT(arg)

// Record a COM object result and return it (wraps in proxy and records)
// Usage: return RECORD_COM_RESULT(actualModule);
#define RECORD_COM_RESULT(result)                       \
    [&]() -> decltype(result)                           \
    {                                                   \
        auto* _wrapped = result;                        \
        _ctx.record(RecordFlag::ReturnValue, _wrapped); \
        return _wrapped;                                \
    }()

// Record return value and return it
#define RECORD_RETURN(result)                     \
    _ctx.record(RecordFlag::ReturnValue, result); \
    return result

// For void returns
#define RECORD_RETURN_VOID() ((void)0)

// =============================================================================
// addRef/release recording macros for proxies
// These record user ref-count changes so playback can match object lifetimes
// =============================================================================

// Override addRef to record the call
// ProxyType is the concrete proxy class (e.g., SessionProxy)
#define PROXY_ADDREF_IMPL(ProxyType)                      \
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override \
    {                                                     \
        if (SlangRecord::isRefCountRecordingSuppressed()) \
            return ProxyBase::addRefImpl();               \
        RECORD_CALL();                                    \
        uint32_t result = ProxyBase::addRefImpl();        \
        RECORD_RETURN(result);                            \
    }

// Override release to record the call
#define PROXY_RELEASE_IMPL(ProxyType)                      \
    SLANG_NO_THROW uint32_t SLANG_MCALL release() override \
    {                                                      \
        if (SlangRecord::isRefCountRecordingSuppressed())  \
            return ProxyBase::releaseImpl();               \
        RECORD_CALL();                                     \
        uint32_t result = ProxyBase::releaseImpl();        \
        RECORD_RETURN(result);                             \
    }

// Convenience macro to add both overrides
#define PROXY_REFCOUNT_IMPL(ProxyType) \
    PROXY_ADDREF_IMPL(ProxyType)       \
    PROXY_RELEASE_IMPL(ProxyType)

// =============================================================================
// Helper templates for variadic input recording
// =============================================================================

// Records all arguments as inputs using fold expression
template<typename... Args>
inline void recordInputs(ReplayContext& ctx, Args&... args)
{
    (ctx.record(RecordFlag::Input, args), ...);
}

// Overload for empty args
inline void recordInputs(ReplayContext&) {}

// =============================================================================
// Higher-level Macros - Common patterns
// =============================================================================

// Pattern: SlangResult method(inputs..., T** outObject)
// Usage: RECORD_METHOD_OUTPUT(createSession, outSession, desc)
//        - outSession is the T** output parameter
//        - desc, etc. are the input parameters
#define RECORD_METHOD_OUTPUT(method, outParam, ...)                         \
    RECORD_CALL();                                                          \
    recordInputs(_ctx, __VA_ARGS__);                                        \
    auto _result = ProxyBase::getActual() -> method(__VA_ARGS__, outParam); \
    RECORD_COM_OUTPUT(outParam);                                            \
    RECORD_RETURN(_result)

// Pattern: T method(inputs...) - method with inputs and return value
#define RECORD_METHOD_RETURN(method, ...)                         \
    RECORD_CALL();                                                \
    recordInputs(_ctx, __VA_ARGS__);                              \
    auto _result = ProxyBase::getActual() -> method(__VA_ARGS__); \
    RECORD_RETURN(_result)

// Pattern: void method(inputs...) - void method with only inputs
#define RECORD_METHOD_VOID(method, ...)          \
    RECORD_CALL();                               \
    recordInputs(_ctx, __VA_ARGS__);             \
    ProxyBase::getActual()->method(__VA_ARGS__); \
    RECORD_RETURN_VOID()

// Pattern: T method() - no-arg method with return value
#define RECORD_METHOD_RETURN_NOARGS(method)            \
    RECORD_CALL();                                     \
    auto _result = ProxyBase::getActual() -> method(); \
    RECORD_RETURN(_result)

// =============================================================================
// Session-specific Macros
// =============================================================================

// Pattern: Return a module from the session, wrapping it in a proxy and keeping
// a reference in m_loadedModules to manage its lifetime. The proxy created by
// wrapObject already has refcount 1 so we use INIT_ATTACH to avoid double addRef.
// Requires: RECORD_CALL() has been called (provides _ctx), and method has m_loadedModules.
// Usage: RECORD_RETURN_SESSION(result)  where result is slang::IModule*
#define RECORD_RETURN_SESSION(result)                                                       \
    {                                                                                       \
        uint64_t _handle = kNullHandle;                                                     \
        if (result)                                                                         \
        {                                                                                   \
            SuppressRefCountRecording _guard;                                               \
            result = wrapObject(result);                                                    \
            m_loadedModules.add(Slang::ComPtr<slang::IModule>(Slang::INIT_ATTACH, result)); \
            _handle = _ctx.getProxyHandle(result);                                          \
        }                                                                                   \
        _ctx.recordHandle(RecordFlag::ReturnValue, _handle);                                \
        return result;                                                                      \
    }

// Pattern: Return a COM object that is already known to have a proxy registered.
// Uses getProxy instead of wrapObject — the proxy must already exist.
// Requires: RECORD_CALL() has been called (provides _ctx).
// Usage: RECORD_RETURN_EXISTING_PROXY(result)
#define RECORD_RETURN_EXISTING_PROXY(result)                                            \
    {                                                                                   \
        uint64_t _handle = kNullHandle;                                                 \
        if (result)                                                                     \
        {                                                                               \
            auto* _proxy = _ctx.getProxy(result);                                       \
            result = toSlangInterface<std::remove_pointer_t<decltype(result)>>(_proxy); \
            _handle = _ctx.getProxyHandle(_proxy);                                      \
        }                                                                               \
        _ctx.recordHandle(RecordFlag::ReturnValue, _handle);                            \
        return result;                                                                  \
    }

// =============================================================================
// Playback Registration Macros
// =============================================================================

// Helper to get default-initialized value for a type
template<typename T>
struct DefaultValue
{
    static T get() { return T{}; }
};

// Specialization for pointer types - return nullptr
template<typename T>
struct DefaultValue<T*>
{
    static T* get() { return nullptr; }
};

// Specialization for reference types - need static storage
template<typename T>
struct DefaultValue<T&>
{
    static T& get()
    {
        static T value{};
        return value;
    }
};

template<typename T>
struct DefaultValue<const T&>
{
    static const T& get()
    {
        static T value{};
        return value;
    }
};

// =============================================================================
// Function traits to extract return type and argument types from member functions
// =============================================================================


template<typename T>
struct MemberFunctionTraits;

// Non-const member function
template<typename R, typename C, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...)>
{
    using ReturnType = R;
    using ClassType = C;
    using ArgsTuple = std::tuple<Args...>;
    static constexpr size_t Arity = sizeof...(Args);
};

// Const member function
template<typename R, typename C, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) const>
{
    using ReturnType = R;
    using ClassType = C;
    using ArgsTuple = std::tuple<Args...>;
    static constexpr size_t Arity = sizeof...(Args);
};

// Non-const member function
template<typename R, typename C, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) noexcept>
{
    using ReturnType = R;
    using ClassType = C;
    using ArgsTuple = std::tuple<Args...>;
    static constexpr size_t Arity = sizeof...(Args);
};

// Const member function
template<typename R, typename C, typename... Args>
struct MemberFunctionTraits<R (C::*)(Args...) const noexcept>
{
    using ReturnType = R;
    using ClassType = C;
    using ArgsTuple = std::tuple<Args...>;
    static constexpr size_t Arity = sizeof...(Args);
};

// =============================================================================
// Replay caller - calls a method with default arguments
// =============================================================================

template<typename MethodPtr, typename ProxyType, typename... Args, size_t... Is>
auto callWithDefaults(ProxyType* proxy, MethodPtr method, std::index_sequence<Is...>) ->
    typename MemberFunctionTraits<MethodPtr>::ReturnType
{
    using Traits = MemberFunctionTraits<MethodPtr>;
    using ArgsTuple = typename Traits::ArgsTuple;

    // Call the method with default-initialized arguments
    // The proxy method will read actual values from the replay stream
    return (proxy->*method)(DefaultValue<std::tuple_element_t<Is, ArgsTuple>>::get()...);
}

template<typename MethodPtr, typename ProxyType>
auto callMethodWithDefaults(ProxyType* proxy, MethodPtr method) ->
    typename MemberFunctionTraits<MethodPtr>::ReturnType
{
    using Traits = MemberFunctionTraits<MethodPtr>;
    return callWithDefaults<MethodPtr, ProxyType>(
        proxy,
        method,
        std::make_index_sequence<Traits::Arity>{});
}

// Void return type specialization
template<typename MethodPtr, typename ProxyType, size_t... Is>
void callWithDefaultsVoid(ProxyType* proxy, MethodPtr method, std::index_sequence<Is...>)
{
    using Traits = MemberFunctionTraits<MethodPtr>;
    using ArgsTuple = typename Traits::ArgsTuple;

    (proxy->*method)(DefaultValue<std::tuple_element_t<Is, ArgsTuple>>::get()...);
}

template<typename MethodPtr, typename ProxyType>
void callMethodWithDefaultsVoid(ProxyType* proxy, MethodPtr method)
{
    using Traits = MemberFunctionTraits<MethodPtr>;
    callWithDefaultsVoid<MethodPtr, ProxyType>(
        proxy,
        method,
        std::make_index_sequence<Traits::Arity>{});
}

// =============================================================================
// Replay handler generator
// =============================================================================

// Generate a replay handler for a method that returns a value
template<typename InterfaceType, typename ProxyType, typename MethodPtr>
void replayHandler(ReplayContext& ctx, MethodPtr method)
{
    // Get 'this' pointer from the context (already read by executeNextCall)
    auto* proxy = ctx.getCurrentThis<ProxyType>();
    if (!proxy)
    {
        throw Slang::Exception("Replay: null 'this' pointer");
    }

    // Call the method with default args - the proxy will read from stream
    using Traits = MemberFunctionTraits<MethodPtr>;
    if constexpr (std::is_void_v<typename Traits::ReturnType>)
    {
        callMethodWithDefaultsVoid(proxy, method);
    }
    else
    {
        callMethodWithDefaults(proxy, method);
    }
}

// =============================================================================
// Registration macro
// =============================================================================

// Register a replay handler for a proxy method
// Usage: REPLAY_REGISTER(GlobalSessionProxy, findProfile)
//
// This creates a static handler function and registers it with the replay context.
// The signature is normalized to "ProxyType::methodName" format to match what
// parseSignature extracts from __FUNCSIG__.

#define REPLAY_REGISTER(ProxyType, methodName)                                      \
    do                                                                              \
    {                                                                               \
        /* Create a handler that captures the method pointer */                     \
        static auto handler = [](ReplayContext& ctx)                                \
        { replayHandler<ProxyType, ProxyType>(ctx, &ProxyType::methodName); };      \
        /* Register with normalized signature "ProxyType::methodName" */            \
        ReplayContext::get().registerHandler(#ProxyType "::" #methodName, handler); \
    } while (0)

} // namespace SlangRecord
