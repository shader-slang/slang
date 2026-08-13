// slang-memory-report.h
#pragma once

#include "core/slang-basic.h"

namespace Slang
{

class Linkage;

/// A per-component account, in bytes, of the memory the compiler is holding.
///
/// Each arena-backed component carries two numbers because they answer different questions.
/// `Used` is the payload the component asked for; `Reserved` is what the arena took from the OS to
/// satisfy those requests. Only `Reserved` is resident, and the gap between the two is real process
/// memory that no component's payload explains — `ASTBuilder` arenas allocate in 2 MiB blocks while
/// `IRModule` arenas use 16 KiB blocks, so the slack differs sharply between the two and a single
/// "AST is N bytes" number would hide it.
struct MemoryReport
{
    size_t astArenaUsed = 0;
    size_t astArenaReserved = 0;
    size_t irArenaUsed = 0;
    size_t irArenaReserved = 0;
    size_t sourceArenaUsed = 0;
    size_t sourceArenaReserved = 0;

    /// Retained text of every loaded source file, which the arenas above do not own.
    size_t sourceContent = 0;
};

/// Return what `linkage` and its global session are holding at the instant of the call.
///
/// The result is an instantaneous account, not a high-water mark. Taken after a compile finishes it
/// describes what is still live, which is a lower bound on the process peak: memory that was
/// allocated and freed mid-compile shaped the peak but is gone by the time this runs.
MemoryReport captureMemoryReport(Linkage* linkage);

/// Append `report` as `[MEM] <counter>Kb\t<n>kb` lines.
///
/// The format is the one `tools/compile-perf/bench.py` (`parse_mem`) already parses from the api
/// driver, so counters emitted here reach the tracked series without any change on the tracker
/// side. Two suffix conventions meet in that line and both are load-bearing: the counter name ends
/// in a capital `Kb`, which is what classifies it as kilobytes rather than milliseconds for
/// display and for the trend gate, while the value ends in a lowercase `kb`.
void appendMemoryReportLines(const MemoryReport& report, StringBuilder& out);

} // namespace Slang
