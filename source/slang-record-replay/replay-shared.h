#pragma once

#include <slang.h>
#include "slang-com-ptr.h"

namespace SlangRecord
{

#ifndef SLANG_FUNC_SIG
#ifdef __FUNCSIG__
#define SLANG_FUNC_SIG __FUNCSIG__
#elif defined(__PRETTY_FUNCTION__)
#define SLANG_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__FUNCTION__)
#define SLANG_FUNC_SIG __FUNCTION__
#else
#define SLANG_FUNC_SIG "UnknownFunction"
#endif
#endif

// =============================================================================
// Ref-count recording suppression
// =============================================================================
// Thread-local counter that, when > 0, tells proxy addRef/release macros
// to skip recording and just forward to the underlying ref-count.
// This allows internal code (e.g. queryInterface-based identity queries)
// to manipulate ref-counts without polluting the replay stream.
//
// Use the RAII guard `SuppressRefCountRecording` rather than manipulating
// the counter directly.

/// Access the raw suppression counter (for internal use by the RAII guard).
inline int& suppressionCounter()
{
    thread_local int counter = 0;
    return counter;
}

/// Check if ref-count recording is currently suppressed on this thread.
inline bool isRefCountRecordingSuppressed()
{
    return suppressionCounter() > 0;
}

/// RAII guard that suppresses ref-count recording for its lifetime.
/// Nestable — uses a counter, not a boolean.
struct SuppressRefCountRecording
{
    SuppressRefCountRecording() { ++suppressionCounter(); }
    ~SuppressRefCountRecording() { --suppressionCounter(); }

    // Non-copyable, non-movable
    SuppressRefCountRecording(const SuppressRefCountRecording&) = delete;
    SuppressRefCountRecording& operator=(const SuppressRefCountRecording&) = delete;
};

// =============================================================================
// Canonical identity helpers using queryInterface
// =============================================================================

/// Get the canonical ISlangUnknown* identity for any COM object.
/// Uses queryInterface(ISlangUnknown) to get a consistent pointer regardless
/// of which interface or inheritance path the input pointer came through.
/// Suppresses ref-count recording so proxy addRef/release are not logged.
/// Returns nullptr if obj is nullptr or queryInterface fails.
template<typename T>
inline ISlangUnknown* toSlangUnknown(T* obj)
{
    if (!obj)
        return nullptr;

    SuppressRefCountRecording guard;

    ISlangUnknown* result = nullptr;
    // queryInterface will addRef the returned pointer
    obj->queryInterface(ISlangUnknown::getTypeGuid(), (void**)&result);
    // We only want the identity — undo the addRef
    if (result)
        result->release();
    return result;
}

/// Query a COM object for a specific interface T.
/// Returns a raw pointer (no ownership transfer) or nullptr if unsupported.
/// Suppresses ref-count recording so proxy addRef/release are not logged.
template<typename T>
inline T* toSlangInterface(ISlangUnknown* obj)
{
    if (!obj)
        return nullptr;

    SuppressRefCountRecording guard;

    T* result = nullptr;
    obj->queryInterface(T::getTypeGuid(), (void**)&result);
    // We only want the pointer — undo the addRef
    if (result)
        result->release();
    return result;
}

} // namespace SlangRecord
