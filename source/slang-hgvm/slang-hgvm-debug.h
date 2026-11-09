#pragma once

#include "core/slang-basic.h"
#include "slang-hgbc-format.h"

#include <atomic>

namespace Slang::HGVM
{
/// Describes a native object's read-only diagnostic property, without exposing its host pointer.
struct DebugObjectProperty
{
    String name;
    String value;
};

/// Describes buffer elements without reading device memory. Host stride describes the converted
/// bytes returned to the debugger; device stride describes the actual allocation.
struct DebugBufferInfo
{
    HGBCId elementType = 0;
    uint32_t elementCount = 0;
    uint32_t hostStride = 0;
    uint32_t deviceStride = 0;
};

/// Bounds a stopped session's buffer snapshots and any individual interpreter readback request.
inline constexpr uint64_t kMaxDebugBufferBytes = 64ull * 1024 * 1024;

/// Identifies a record in one interpreter-owned staged program, in 32-bit words.
struct DebugInstructionAddress
{
    uint64_t programId = 0;
    uint32_t wordOffset = 0;
};

/// Identifies one active invocation without exposing its frame storage. Never reused.
struct DebugCallId
{
    uint64_t value = 0;
};

enum class StepAction
{
    /// Executes the current source instruction and stops at the next instruction boundary.
    Step,
    /// Runs until an instruction with its breakpoint bit set is reached.
    Continue,
    /// Unwinds managed values without executing more user instructions.
    Stop,
};

/// Borrows execution control for a synchronous invocation. Callbacks must not throw or reenter it.
/// Called before the first instruction and each subsequent stop, before that instruction executes.
/// Scope directives update the debug handler before a stop; generated cleanup is not interruptible
/// during cancellation or failure. Continuing from a breakpoint executes it once before rearming.
/// Lowered copy and retain records complete together before any control callback observes them.
/// breakpointHit reports the current record's break bit, including at entry or during stepping.
class IStepControl
{
public:
    virtual ~IStepControl() = default;
    virtual StepAction onStep(DebugInstructionAddress address, bool breakpointHit) = 0;

    /// Requests a callback at the next complete instruction boundary. Safe from another thread.
    void requestControl() { m_controlPending.store(true, std::memory_order_release); }
    bool takeControlRequest()
    {
        return m_controlPending.load(std::memory_order_relaxed) &&
               m_controlPending.exchange(false, std::memory_order_acquire);
    }
    virtual StepAction onControl(
        DebugInstructionAddress address,
        StepAction action,
        bool breakpointHit)
    {
        return action == StepAction::Step || breakpointHit ? onStep(address, breakpointHit)
                                                           : action;
    }

private:
    std::atomic<bool> m_controlPending{false};
};

/// Reports compact execution context without exposing the interpreter's frame representation.
/// Scope indices describe immutable debug-map entries. A call saves the caller's current context;
/// return restores it after cleanup, including when execution is cancelled or fails.
/// A scope update replaces the active scope, rather than pushing a frame; scope zero means unknown.
/// The inline-site count is staged with the scope index so depth tracking needs no metadata lookup.
/// For the entry-point call, onFunctionEnter receives its entry instruction instead of a call site.
class IDebugHandler
{
public:
    virtual ~IDebugHandler() = default;
    virtual void onFunctionEnter(DebugInstructionAddress callSite, DebugCallId call) = 0;
    virtual void onFunctionExit() = 0;
    virtual void onScope(uint32_t scopeIndex, uint32_t inlineSiteCount) = 0;
};
} // namespace Slang::HGVM
