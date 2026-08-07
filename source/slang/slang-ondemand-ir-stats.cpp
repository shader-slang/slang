// slang-ondemand-ir-stats.cpp
#include "slang-ondemand-ir-stats.h"

#include "core/slang-dictionary.h"
#include "core/slang-list.h"

#include <cstdio>
#include <cstdlib>
#include <mutex>

namespace Slang
{
namespace OnDemandStats
{

namespace
{

struct Registry
{
    std::mutex mutex;
    List<PhaseRecord> phases;
    List<IRModuleShape> shapes;
    List<IRSubPhases> subPhases;
    // Distinct mangled names resolved per module, so a symbol pulled repeatedly
    // counts once. The interesting figure is coverage, not traffic.
    Dictionary<String, Dictionary<String, bool>> usedSymbolsByModule;
    // Distinct global values cloned out of each module, and the total size of
    // their instruction subtrees.
    Dictionary<String, Dictionary<const void*, int64_t>> clonedByModule;
    Dictionary<String, int64_t> clonedInstTotalByModule;
    uint64_t baselineRSS = 0;
    bool dumped = false;
};

Registry& getRegistry()
{
    static Registry registry;
    return registry;
}

} // namespace

bool isEnabled()
{
    static const bool enabled = []
    {
        const char* value = ::getenv("SLANG_ONDEMAND_STATS");
        const bool on = value && value[0] != '\0' && value[0] != '0';
        if (on)
        {
            getRegistry().baselineRSS = getCurrentRSSBytes();
            ::atexit([] { dumpReport(); });
        }
        return on;
    }();
    return enabled;
}

uint64_t getCurrentRSSBytes()
{
#if defined(__linux__)
    // /proc/self/statm reports sizes in pages; field 2 is resident.
    FILE* file = ::fopen("/proc/self/statm", "r");
    if (!file)
        return 0;
    unsigned long total = 0;
    unsigned long resident = 0;
    const int read = ::fscanf(file, "%lu %lu", &total, &resident);
    ::fclose(file);
    if (read != 2)
        return 0;
    return uint64_t(resident) * uint64_t(::sysconf(_SC_PAGESIZE));
#else
    return 0;
#endif
}

void recordPhase(const PhaseRecord& record)
{
    if (!isEnabled())
        return;
    Registry& registry = getRegistry();
    std::lock_guard<std::mutex> lock(registry.mutex);
    registry.phases.add(record);
}

void recordIRSubPhases(const IRSubPhases& phases)
{
    if (!isEnabled())
        return;
    Registry& registry = getRegistry();
    std::lock_guard<std::mutex> lock(registry.mutex);
    registry.subPhases.add(phases);
}

void recordIRModuleShape(const IRModuleShape& shape)
{
    if (!isEnabled())
        return;
    Registry& registry = getRegistry();
    std::lock_guard<std::mutex> lock(registry.mutex);
    registry.shapes.add(shape);
}

void completeLastIRModuleShape(
    const String& moduleName,
    int64_t serializedByteCount,
    int64_t arenaBytesUsed)
{
    if (!isEnabled())
        return;
    Registry& registry = getRegistry();
    std::lock_guard<std::mutex> lock(registry.mutex);
    if (registry.shapes.getCount() == 0)
        return;
    auto& shape = registry.shapes.getLast();
    shape.moduleName = moduleName;
    shape.serializedByteCount = serializedByteCount;
    shape.arenaBytesUsed = arenaBytesUsed;
}

void recordSymbolUse(const UnownedStringSlice& mangledName, const char* moduleName)
{
    if (!isEnabled())
        return;
    Registry& registry = getRegistry();
    std::lock_guard<std::mutex> lock(registry.mutex);
    registry.usedSymbolsByModule[String(moduleName)][String(mangledName)] = true;
}

void recordGlobalValueClone(const char* moduleName, const void* inst, int64_t subtreeInstCount)
{
    if (!isEnabled())
        return;
    Registry& registry = getRegistry();
    std::lock_guard<std::mutex> lock(registry.mutex);
    auto& cloned = registry.clonedByModule[String(moduleName)];
    if (cloned.addIfNotExists(inst, subtreeInstCount))
    {
        registry.clonedInstTotalByModule[String(moduleName)] += subtreeInstCount;
    }
}

void dumpReport()
{
    Registry& registry = getRegistry();
    std::lock_guard<std::mutex> lock(registry.mutex);
    if (registry.dumped)
        return;
    registry.dumped = true;

    ::fprintf(stderr, "\n=== on-demand IR stats ===\n");

    ::fprintf(stderr, "\n-- deserialization phases --\n");
    ::fprintf(stderr, "%-6s %-12s %10s %14s\n", "phase", "module", "ms", "rss delta MiB");
    double astMs = 0, irMs = 0;
    int64_t astBytes = 0, irBytes = 0;
    for (const auto& phase : registry.phases)
    {
        ::fprintf(
            stderr,
            "%-6s %-12s %10.2f %14.2f\n",
            phase.phase,
            phase.moduleName.getBuffer(),
            phase.elapsedMs,
            double(phase.rssDeltaBytes) / (1024.0 * 1024.0));
        if (phase.phase[0] == 'A')
        {
            astMs += phase.elapsedMs;
            astBytes += phase.rssDeltaBytes;
        }
        else
        {
            irMs += phase.elapsedMs;
            irBytes += phase.rssDeltaBytes;
        }
    }
    const double totalMs = astMs + irMs;
    const int64_t totalBytes = astBytes + irBytes;
    if (totalMs > 0)
    {
        ::fprintf(
            stderr,
            "TOTAL  AST          %10.2f %14.2f  (%.0f%% time, %.0f%% mem)\n",
            astMs,
            double(astBytes) / (1024.0 * 1024.0),
            100.0 * astMs / totalMs,
            totalBytes ? 100.0 * double(astBytes) / double(totalBytes) : 0.0);
        ::fprintf(
            stderr,
            "TOTAL  IR           %10.2f %14.2f  (%.0f%% time, %.0f%% mem)\n",
            irMs,
            double(irBytes) / (1024.0 * 1024.0),
            100.0 * irMs / totalMs,
            totalBytes ? 100.0 * double(irBytes) / double(totalBytes) : 0.0);
    }

    ::fprintf(stderr, "\n-- IR module shape --\n");
    for (const auto& shape : registry.shapes)
    {
        const double arenaMiB = double(shape.arenaBytesUsed) / (1024.0 * 1024.0);
        const double serializedMiB = double(shape.serializedByteCount) / (1024.0 * 1024.0);
        ::fprintf(
            stderr,
            "%s: %lld insts (%lld global), %lld operand slots, %lld string bytes, %lld literals\n",
            shape.moduleName.getBuffer(),
            (long long)shape.instCount,
            (long long)shape.globalInstCount,
            (long long)shape.operandSlotCount,
            (long long)shape.stringByteCount,
            (long long)shape.literalCount);
        ::fprintf(
            stderr,
            "%s: arena %.2f MiB, serialized %.2f MiB, expansion %.2fx, %.0f bytes/inst\n",
            shape.moduleName.getBuffer(),
            arenaMiB,
            serializedMiB,
            serializedMiB > 0 ? arenaMiB / serializedMiB : 0.0,
            shape.instCount ? double(shape.arenaBytesUsed) / double(shape.instCount) : 0.0);
        ::fprintf(
            stderr,
            "%s: symbol-index eager tier would be %lld insts of %lld (%.2f%%)\n",
            shape.moduleName.getBuffer(),
            (long long)shape.eagerTierInstCount,
            (long long)shape.instCount,
            shape.instCount ? 100.0 * double(shape.eagerTierInstCount) / double(shape.instCount)
                            : 0.0);
    }

    ::fprintf(stderr, "\n-- inside IR deserialization --\n");
    for (const auto& p : registry.subPhases)
    {
        const double total = p.copyFlatTableMs + p.allocInstsMs + p.wireUpMs;
        ::fprintf(
            stderr,
            "copy flat table %.2f ms (%.0f%%, %.2f MiB) | alloc insts %.2f ms (%.0f%%, %.2f MiB)"
            " | wire up %.2f ms (%.0f%%)\n",
            p.copyFlatTableMs,
            total ? 100.0 * p.copyFlatTableMs / total : 0.0,
            double(p.copyFlatTableRSSDelta) / (1024.0 * 1024.0),
            p.allocInstsMs,
            total ? 100.0 * p.allocInstsMs / total : 0.0,
            double(p.allocInstsRSSDelta) / (1024.0 * 1024.0),
            p.wireUpMs,
            total ? 100.0 * p.wireUpMs / total : 0.0);
    }

    ::fprintf(stderr, "\n-- symbols resolved out of serialized modules --\n");
    if (registry.usedSymbolsByModule.getCount() == 0)
    {
        ::fprintf(stderr, "(none)\n");
    }
    for (const auto& [moduleName, symbols] : registry.usedSymbolsByModule)
    {
        int64_t globalInsts = 0;
        for (const auto& shape : registry.shapes)
        {
            if (shape.moduleName == moduleName)
                globalInsts = shape.globalInstCount;
        }
        ::fprintf(
            stderr,
            "%s: %d distinct symbols resolved",
            moduleName.getBuffer(),
            (int)symbols.getCount());
        if (globalInsts > 0)
        {
            ::fprintf(
                stderr,
                " of %lld global insts (%.1f%%)",
                (long long)globalInsts,
                100.0 * double(symbols.getCount()) / double(globalInsts));
        }
        ::fprintf(stderr, "\n");
    }

    ::fprintf(stderr, "\n-- global values cloned out of serialized modules --\n");
    if (registry.clonedByModule.getCount() == 0)
    {
        ::fprintf(stderr, "(none)\n");
    }
    for (const auto& [moduleName, cloned] : registry.clonedByModule)
    {
        int64_t totalInsts = 0;
        int64_t globalInsts = 0;
        for (const auto& shape : registry.shapes)
        {
            if (shape.moduleName == moduleName)
            {
                totalInsts = shape.instCount;
                globalInsts = shape.globalInstCount;
            }
        }
        int64_t subtreeTotal = 0;
        registry.clonedInstTotalByModule.tryGetValue(moduleName, subtreeTotal);
        ::fprintf(
            stderr,
            "%s: %d global values cloned",
            moduleName.getBuffer(),
            (int)cloned.getCount());
        if (globalInsts > 0)
            ::fprintf(
                stderr,
                " of %lld (%.2f%%)",
                (long long)globalInsts,
                100.0 * double(cloned.getCount()) / double(globalInsts));
        ::fprintf(stderr, ", covering %lld insts", (long long)subtreeTotal);
        if (totalInsts > 0)
            ::fprintf(
                stderr,
                " of %lld (%.2f%%)",
                (long long)totalInsts,
                100.0 * double(subtreeTotal) / double(totalInsts));
        ::fprintf(stderr, "\n");
    }

    ::fprintf(
        stderr,
        "\nprocess RSS at exit: %.2f MiB\n\n",
        double(getCurrentRSSBytes()) / (1024.0 * 1024.0));
}

} // namespace OnDemandStats
} // namespace Slang
