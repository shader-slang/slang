#pragma once

#include "../replay-context.h"
#include "proxy-base.h"

namespace SlangRecord {

// =============================================================================
// Core Macros - Fine-grained control for recording API calls
// =============================================================================

// Begins a recorded instance method call - locks context, records signature and 'this'
// Uses compiler-specific macro for full signature including type name
#ifdef _MSC_VER
#define RECORD_CALL() \
    auto& _ctx = ReplayContext::get(); \
    auto _lock = _ctx.lock(); \
    _ctx.beginCall(__FUNCSIG__, this)
#else
#define RECORD_CALL() \
    auto& _ctx = ReplayContext::get(); \
    auto _lock = _ctx.lock(); \
    _ctx.beginCall(__PRETTY_FUNCTION__, this)
#endif

// For static/free functions (no 'this' pointer)
#ifdef _MSC_VER
#define RECORD_STATIC_CALL() \
    auto& _ctx = ReplayContext::get(); \
    auto _lock = _ctx.lock(); \
    _ctx.beginStaticCall(__FUNCSIG__)
#else
#define RECORD_STATIC_CALL() \
    auto& _ctx = ReplayContext::get(); \
    auto _lock = _ctx.lock(); \
    _ctx.beginStaticCall(__PRETTY_FUNCTION__)
#endif

// Record an input parameter
// Note: We cast away const for inputs since recording only reads the value
#define RECORD_INPUT(arg) \
    _ctx.record(RecordFlag::Input, const_cast<std::remove_const_t<std::remove_reference_t<decltype(arg)>>&>(arg))

// Record an output parameter (for T** style outputs, dereferences and wraps)
#define RECORD_OUTPUT(arg) \
    if (arg && *arg) *arg = static_cast<std::remove_pointer_t<decltype(arg)>>(wrapObject(static_cast<ISlangUnknown*>(*arg))); \
    _ctx.record(RecordFlag::Output, *arg)

// Record return value and return it  
#define RECORD_RETURN(result) \
    _ctx.record(RecordFlag::ReturnValue, result); \
    return result

// For void returns
#define RECORD_RETURN_VOID() ((void)0)

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
#define RECORD_METHOD_OUTPUT(method, outParam, ...) \
    RECORD_CALL(); \
    recordInputs(_ctx, __VA_ARGS__); \
    auto _result = ProxyBase::getActual()->method(__VA_ARGS__, outParam); \
    RECORD_OUTPUT(outParam); \
    RECORD_RETURN(_result)

// Pattern: T method(inputs...) - method with inputs and return value
#define RECORD_METHOD_RETURN(method, ...) \
    RECORD_CALL(); \
    recordInputs(_ctx, __VA_ARGS__); \
    auto _result = ProxyBase::getActual()->method(__VA_ARGS__); \
    RECORD_RETURN(_result)

// Pattern: void method(inputs...) - void method with only inputs  
#define RECORD_METHOD_VOID(method, ...) \
    RECORD_CALL(); \
    recordInputs(_ctx, __VA_ARGS__); \
    ProxyBase::getActual()->method(__VA_ARGS__); \
    RECORD_RETURN_VOID()

// Pattern: T method() - no-arg method with return value
#define RECORD_METHOD_RETURN_NOARGS(method) \
    RECORD_CALL(); \
    auto _result = ProxyBase::getActual()->method(); \
    RECORD_RETURN(_result)

} // namespace SlangRecord
