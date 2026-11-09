#pragma once

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
} // namespace Slang::HGVM
