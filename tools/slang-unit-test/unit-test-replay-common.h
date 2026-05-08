// unit-test-replay-common.h
// Common includes and helpers for replay unit tests

#pragma once

// Include cpp files directly to access internal symbols not exported from slang DLL
#include "../../source/core/slang-file-system.h"
#include "../../source/core/slang-io.h"
#include "../../source/slang-record-replay/proxy/proxy-base.h"
#include "../../source/slang-record-replay/proxy/proxy-global-session.h"
#include "../../source/slang-record-replay/replay-context.h"
#include "unit-test/slang-unit-test.h"

#include <cstring>

using namespace Slang;
using namespace SlangRecord;

inline ReplayContext& ctx()
{
    return ReplayContext::get();
}

class ScopedReplayContext
{
public:
    ScopedReplayContext() { ctx().reset(); }

    ~ScopedReplayContext() { ctx().reset(); }
};


// Force the singleton ReplayContext back to a clean state on entry and exit so
// the test sees Mode::Idle regardless of how the previous test in this process
// finished (including aborts or assertion paths that skipped the dtor).
#define REPLAY_TEST ScopedReplayContext _scopedReplayContext;

// =============================================================================
// Helper: Round-trip test template
// Writes a value, creates a reader, reads it back, and compares
// =============================================================================

template<typename T>
static bool roundTripValue(T writeValue, T& readValue)
{
    ctx().reset();
    ctx().setMode(Mode::Record);
    ctx().record(RecordFlag::None, writeValue);

    ctx().switchToPlayback();
    ctx().record(RecordFlag::None, readValue);

    return ctx().getStream().atEnd();
}

template<typename T>
static bool roundTripCheck(T value)
{
    T readValue{};
    bool atEnd = roundTripValue(value, readValue);
    return atEnd && (readValue == value);
}
