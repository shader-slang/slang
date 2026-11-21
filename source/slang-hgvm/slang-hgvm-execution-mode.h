#pragma once

#include <stdint.h>

namespace Slang::HGVM
{
/// Selects when recorded GPU work is submitted and completed. Default is buffered for an
/// interpreter and synchronous for a DebugSession; an explicit selection overrides either.
enum class GPUExecutionMode
{
    Default,
    Buffered,
    Synchronous,
};

/// Selects the storage strategy behind a `FrameStack`.
///
/// `Default` resolves to `Reserved` on native hosts and to `Segmented` on WebAssembly, where the
/// VM cannot reserve address space without committing it and where several suspended execution
/// contexts may each need their own stack.
enum class FrameStackKind : uint8_t
{
    Default,
    Reserved,
    Segmented,
};
} // namespace Slang::HGVM
