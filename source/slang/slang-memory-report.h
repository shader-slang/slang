// slang-memory-report.h
#pragma once

#include "core/slang-basic.h"

namespace Slang
{

class FrontEndCompileRequest;
class Linkage;

/// Which modules a component's memory belongs to.
///
/// A module is counted once, under the first category that reaches it, so the traversal order in
/// `captureMemoryReport` is the definition of these buckets rather than an implementation detail:
/// builtin modules are walked first, so a builtin imported by the compiled source is charged to
/// `Builtin` and not to `User`.
enum class ModuleCategory
{
    /// The core module and the other builtin modules (GLSL, autodiff), loaded once per session.
    /// This is the "what does a session cost" number, and on an empty-shader compile it is
    /// essentially the whole footprint.
    Builtin,

    /// Modules loaded from the source being compiled, and the modules under compilation.
    User,

    // There is deliberately no category for the linked and specialized IR built during code
    // generation. That IR is transient — released once target code has been emitted — so an
    // account taken after the compile finishes cannot see it, and a counter for it would report a
    // constant zero that reads as "code generation costs no memory". Measuring it needs sampling
    // at the peak rather than at the end; see tools/compile-perf/DESIGN.md.

    CountOf,
};

/// A per-component account, in bytes, of the memory the compiler is holding.
///
/// Each arena-backed component carries two numbers because they answer different questions.
/// `Used` is the payload the component asked for; `Reserved` is what the arena obtained from the
/// allocator to satisfy those requests. The gap between them is block slack — `ASTBuilder` arenas
/// allocate in 2 MiB blocks while `IRModule` arenas use 16 KiB ones, so a single "AST is N bytes"
/// number would hide a difference that is real process memory.
///
/// `Reserved` is an UPPER BOUND on a component's residency, not its residency: pages inside a
/// reserved block that have not been written are not resident. On an empty-shader compile the AST
/// arena reserves 8.0 MiB against 5.6 MiB used, so up to 2.4 MiB of what is attributed may never
/// appear in the process total. This is why the residual below is computed with a floor rather
/// than allowed to go negative.
struct MemoryReport
{
    size_t astArenaUsed = 0;
    size_t astArenaReserved = 0;
    size_t irArenaUsed = 0;
    size_t irArenaReserved = 0;
    size_t sourceArenaUsed = 0;
    size_t sourceArenaReserved = 0;

    /// `astArenaReserved` and `irArenaReserved` split by which modules own them. Each array sums to
    /// its aggregate above; the split says whether a change came from the session's fixed cost or
    /// from the compile.
    size_t astArenaReservedBy[size_t(ModuleCategory::CountOf)] = {};
    size_t irArenaReservedBy[size_t(ModuleCategory::CountOf)] = {};

    /// Retained text of every loaded source file, which the arenas above do not own.
    size_t sourceContent = 0;

    /// IR lookup tables that live on the heap rather than in any `IRModule` arena — the
    /// deduplication context's maps and the modules' own side maps. Part of IR's cost, counted
    /// apart from `irArena*` because it scales with instruction count independently of the
    /// instructions' own bytes.
    size_t irSideTables = 0;

    /// The process's resident set at the instant this report was taken, or 0 if the platform
    /// reader failed. This is what makes the rest of the report interpretable: without a total
    /// captured at the SAME moment, the components could only be compared against a peak measured
    /// over the whole process lifetime, and the difference between the two would conflate
    /// "memory we have not attributed" with "memory that was already freed".
    size_t processRss = 0;
};

/// Return what `linkage`, its global session, and `frontEndReq` are holding at the instant of the
/// call.
///
/// `frontEndReq` is not optional for a correct report, even though the walk tolerates null. The
/// module being compiled belongs to a `TranslationUnitRequest`, not to the linkage:
/// `linkage->loadedModulesList` holds what the source `import`ed, so a linkage-only walk sees the
/// core module and every dependency but not the code under compilation, and reports the compile's
/// own IR as zero.
///
/// The result is an instantaneous account, not a high-water mark. Taken after a compile finishes it
/// describes what is still live, which is a lower bound on the process peak: memory that was
/// allocated and freed mid-compile shaped the peak but is gone by the time this runs.
MemoryReport captureMemoryReport(Linkage* linkage, FrontEndCompileRequest* frontEndReq);

/// Append `report` as `[MEM] <counter>Kb\t<n>kb` lines.
///
/// The format is the one `tools/compile-perf/bench.py` (`parse_mem`) already parses from the api
/// driver, so counters emitted here reach the tracked series without any change on the tracker
/// side. Two suffix conventions meet in that line and both are load-bearing: the counter name ends
/// in a capital `Kb`, which is what classifies it as kilobytes rather than milliseconds for
/// display and for the trend gate, while the value ends in a lowercase `kb`.
void appendMemoryReportLines(const MemoryReport& report, StringBuilder& out);

} // namespace Slang
