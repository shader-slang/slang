// slang-memory-report.cpp
#include "slang-memory-report.h"

#include "compiler-core/slang-source-loc.h"
#include "slang-ast-builder.h"
#include "slang-global-session.h"
#include "slang-ir.h"
#include "slang-module.h"
#include "slang-session.h"

#if defined(_WIN32)
#include <psapi.h>
#include <windows.h>
#ifdef _MSC_VER
// Link psapi where the symbol lives, following slang-win-visual-studio-util.cpp's use of the same
// pragma for advapi32/Shell32 — this keeps a single-caller platform dependency out of the build
// files.
#pragma comment(lib, "psapi")
#endif
#elif defined(__APPLE__)
#include <mach/mach.h>
#else
#include <cstdio>
#include <unistd.h>
#endif

namespace Slang
{

namespace
{

/// Return the process's current resident set in bytes, or 0 if it cannot be determined.
///
/// Resident memory, deliberately, not virtual size: the question this answers is how much physical
/// memory the compiler is actually costing the machine, which is also what the suite's peak-RSS
/// measurement records, so the two are directly comparable.
///
/// This duplicates the reader in `tools/compile-perf/native/api-driver.cpp` rather than sharing
/// one. The driver is a separate executable that dlopens libslang and must measure processes built
/// from Slang versions predating this file, so there is no build in which one copy could serve
/// both. Any change here should be mirrored there.
size_t currentProcessRssBytes()
{
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS counters;
    counters.cb = sizeof(counters);
    if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)))
        return size_t(counters.WorkingSetSize);
    return 0;
#elif defined(__APPLE__)
    mach_task_basic_info info;
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count) ==
        KERN_SUCCESS)
        return size_t(info.resident_size);
    return 0;
#else
    FILE* file = fopen("/proc/self/statm", "r");
    if (!file)
        return 0;
    long totalPages = 0;
    long residentPages = 0;
    const int fieldsRead = fscanf(file, "%ld %ld", &totalPages, &residentPages);
    fclose(file);
    if (fieldsRead != 2)
        return 0;
    // The second field is resident pages; the first is total program size and would overstate the
    // footprint by counting memory that was never faulted in.
    return size_t(residentPages) * size_t(sysconf(_SC_PAGESIZE));
#endif
}

/// Sums the arenas and blobs reachable from a linkage, counting each underlying object once.
///
/// Consider a compile of a single source file. The core module is reachable both from the global
/// session's `coreModules` list and, once imported, from the linkage's `loadedModulesList`; the
/// `SourceManager` that owns the file's text is shared by every module loaded through that linkage.
/// A walk that followed each path independently would therefore count the two largest components
/// twice, so deduplication here is not an optimization — without it the totals mean nothing.
/// Identity is taken on the owned object itself (the arena, the source file), not on the module or
/// manager that led to it, so two paths reaching one arena collapse regardless of how they got
/// there.
struct MemoryWalker
{
    MemoryReport report;
    HashSet<const void*> visited;

    /// Return true the first time `object` is seen, false on any later visit or for null.
    bool firstVisit(const void* object) { return object && visited.add(object); }

    void addArena(const MemoryArena& arena, size_t& used, size_t& reserved)
    {
        if (!firstVisit(&arena))
            return;
        used += arena.calcTotalMemoryUsed();
        reserved += arena.calcTotalMemoryAllocated();
    }

    void addASTBuilder(ASTBuilder* astBuilder)
    {
        if (!astBuilder)
            return;
        addArena(astBuilder->getMemoryArena(), report.astArenaUsed, report.astArenaReserved);
    }

    void addIRModule(IRModule* irModule)
    {
        if (!irModule)
            return;
        // Guarded by the arena's first-visit check rather than its own, so the side tables are
        // counted exactly when their module's arena is — one `if` cannot drift from the other.
        if (!firstVisit(&irModule->getMemoryArena()))
            return;
        report.irArenaUsed += irModule->getMemoryArena().calcTotalMemoryUsed();
        report.irArenaReserved += irModule->getMemoryArena().calcTotalMemoryAllocated();
        report.irSideTables += irModule->calcSideTableMemoryAllocated();
    }

    void addModule(Module* module)
    {
        if (!module)
            return;
        addASTBuilder(module->getASTBuilder());
        addIRModule(module->getIRModule());
    }

    /// Add everything a linkage owns: its own AST builder, the modules loaded through it, the IR
    /// modules it produced, and its source managers.
    void addLinkage(Linkage* linkage)
    {
        if (!linkage)
            return;
        addASTBuilder(linkage->getASTBuilder());
        for (const RefPtr<LoadedModule>& module : linkage->loadedModulesList)
            addModule(module);
        // IR modules produced by this linkage that no `Module` owns — the linked and specialized
        // clones built during code generation. Counted as IR like any other IR module.
        for (const RefPtr<IRModule>& irModule : linkage->compiledModules)
            addIRModule(irModule);
        addSourceManager(linkage->getSourceManager());
    }

    /// Add a source manager's own arena and the retained text of its files, then its parent.
    ///
    /// The parent chain matters: a linkage's source manager is created with the global session's
    /// builtin manager as its parent, and the core module's text hangs off that parent rather than
    /// off the manager the compile used directly.
    void addSourceManager(SourceManager* sourceManager)
    {
        for (; sourceManager; sourceManager = sourceManager->getParent())
        {
            if (!firstVisit(sourceManager))
                return;
            addArena(
                *sourceManager->getMemoryArena(),
                report.sourceArenaUsed,
                report.sourceArenaReserved);
            for (SourceFile* sourceFile : sourceManager->getSourceFiles())
            {
                if (firstVisit(sourceFile))
                    report.sourceContent += sourceFile->getContentSize();
            }
        }
    }
};

} // namespace

MemoryReport captureMemoryReport(Linkage* linkage)
{
    MemoryWalker walker;
    if (!linkage)
        return walker.report;

    walker.addLinkage(linkage);

    if (Session* globalSession = linkage->getSessionImpl())
    {
        // The builtin linkage is walked as a linkage in its own right, not merely as a source of
        // core modules. `Session::loadBuiltinModule` builds core-module AST into ITS root
        // `ASTBuilder` rather than into a per-module one, so reaching the core modules alone would
        // report a core-module AST of a few megabytes and silently charge the rest to the
        // unattributed remainder.
        walker.addLinkage(globalSession->getBuiltinLinkage());
        for (const RefPtr<Module>& coreModule : globalSession->coreModules)
            walker.addModule(coreModule);
        walker.addSourceManager(&globalSession->builtinSourceManager);
    }

    // Read last, so the total covers everything the walk itself allocated (the visited set) rather
    // than reporting a total from before that allocation and charging the difference to the
    // residual.
    walker.report.processRss = currentProcessRssBytes();

    return walker.report;
}

void appendMemoryReportLines(const MemoryReport& report, StringBuilder& out)
{
    // Round to the nearest kilobyte rather than truncating: a component holding less than 512
    // bytes is genuinely closer to 0 KB than to 1 KB, and truncation would bias every counter
    // consistently downward, which is the direction that hides growth.
    // The cast is required, not stylistic: `size_t` is `unsigned long` here and matches neither
    // the `Int64` nor the `UInt64` stream overload exactly, so the call is ambiguous without it.
    auto emit = [&out](const char* name, size_t bytes)
    { out << "[MEM] " << name << "\t" << UInt64((bytes + 512) / 1024) << "kb\n"; };

    emit("astArenaUsedKb", report.astArenaUsed);
    emit("astArenaReservedKb", report.astArenaReserved);
    emit("irArenaUsedKb", report.irArenaUsed);
    emit("irArenaReservedKb", report.irArenaReserved);
    emit("irSideTablesKb", report.irSideTables);
    emit("sourceArenaUsedKb", report.sourceArenaUsed);
    emit("sourceArenaReservedKb", report.sourceArenaReserved);
    emit("sourceContentKb", report.sourceContent);

    // Emitted only when the platform reader worked. A failed read yields 0, and publishing that
    // would make the residual below read as a large negative clamped to zero — a component-sums-
    // equal-the-total story that happens to be false. Absent is honest; zero is not.
    if (report.processRss == 0)
        return;
    emit("endOfCompileRssKb", report.processRss);

    // The residual is computed here, beside the components, rather than left to the consumer: the
    // subtraction is only correct if it names EVERY component, so deriving it downstream would
    // silently go wrong the next time a component is added here. `reserved` is the term that
    // belongs in it, not `used`, because reserved is what is resident and therefore what the
    // process RSS actually contains.
    const size_t attributed = report.astArenaReserved + report.irArenaReserved +
                              report.irSideTables + report.sourceArenaReserved +
                              report.sourceContent;
    emit("unattributedKb", report.processRss > attributed ? report.processRss - attributed : 0);
}

} // namespace Slang
