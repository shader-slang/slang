// slang-memory-report.cpp
#include "slang-memory-report.h"

#include "compiler-core/slang-source-loc.h"
#include "slang-ast-builder.h"
#include "slang-global-session.h"
#include "slang-ir.h"
#include "slang-module.h"
#include "slang-session.h"

namespace Slang
{

namespace
{

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
        addArena(irModule->getMemoryArena(), report.irArenaUsed, report.irArenaReserved);
    }

    void addModule(Module* module)
    {
        if (!module)
            return;
        addASTBuilder(module->getASTBuilder());
        addIRModule(module->getIRModule());
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

    walker.addASTBuilder(linkage->getASTBuilder());
    for (const RefPtr<LoadedModule>& module : linkage->loadedModulesList)
        walker.addModule(module);

    // IR modules produced by this linkage that no `Module` owns — the linked and specialized
    // clones built during code generation. They are counted as IR like any other IR module.
    for (const RefPtr<IRModule>& irModule : linkage->compiledModules)
        walker.addIRModule(irModule);

    if (Session* globalSession = linkage->getSessionImpl())
    {
        for (const RefPtr<Module>& coreModule : globalSession->coreModules)
            walker.addModule(coreModule);
        walker.addSourceManager(&globalSession->builtinSourceManager);
    }

    walker.addSourceManager(linkage->getSourceManager());

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
    emit("sourceArenaUsedKb", report.sourceArenaUsed);
    emit("sourceArenaReservedKb", report.sourceArenaReserved);
    emit("sourceContentKb", report.sourceContent);
}

} // namespace Slang
