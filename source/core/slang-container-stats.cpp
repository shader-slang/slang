// slang-container-stats.cpp
#include "slang-container-stats.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if SLANG_ENABLE_CONTAINER_STATS

#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#if SLANG_WINDOWS_FAMILY
#include <process.h>
#else
#include <unistd.h>
#endif

#if SLANG_HAS_BACKTRACE
#include <dlfcn.h>
#include <execinfo.h>
#endif

namespace Slang
{

// Nothing in this file may use Slang's own containers or `String`. Those are the very types being
// instrumented, so using one here would make recording a statistic recursively record a statistic.
// The registry therefore uses the standard library throughout, and the report is written with
// `stdio` rather than with `File::writeAllText`.

namespace
{

/// A key identifying a site by the *content* of its file name rather than by pointer.
///
/// The per-thread cache keys on the pointer returned by `__builtin_FILE()`, which is only unique
/// within a translation unit. A header included into two translation units yields two different
/// pointers for the same source line, so the registry compares strings in order to merge them into
/// a single record.
struct SiteKey
{
    std::string file;
    int line;
    const void* typeInfo;

    bool operator==(const SiteKey& rhs) const
    {
        return line == rhs.line && typeInfo == rhs.typeInfo && file == rhs.file;
    }
};

struct SiteKeyHash
{
    size_t operator()(const SiteKey& key) const
    {
        size_t h = std::hash<std::string>()(key.file);
        h ^= std::hash<int>()(key.line) + 0x9E3779B9u + (h << 6) + (h >> 2);
        h ^= std::hash<const void*>()(key.typeInfo) + 0x9E3779B9u + (h << 6) + (h >> 2);
        return h;
    }
};

/// A key identifying a record by the call stack that reached it.
///
/// Only the hash of the stack is kept, not the frames, so two distinct call stacks that collide
/// would merge into one record. With a 64-bit hash that is vanishingly unlikely for the few
/// thousand distinct stacks a compile produces, and the consequence would be a misattributed
/// ranking entry rather than a wrong total.
struct BacktraceKey
{
    uint64_t stackHash;
    const void* typeInfo;

    bool operator==(const BacktraceKey& rhs) const
    {
        return stackHash == rhs.stackHash && typeInfo == rhs.typeInfo;
    }
};

struct BacktraceKeyHash
{
    size_t operator()(const BacktraceKey& key) const
    {
        size_t h = std::hash<uint64_t>()(key.stackHash);
        h ^= std::hash<const void*>()(key.typeInfo) + 0x9E3779B9u + (h << 6) + (h >> 2);
        return h;
    }
};

/// The process-wide set of site records.
///
/// Records must never move, because every live container instance holds a raw pointer to one, so
/// they are allocated from a chunked arena rather than from a growable array. The registry itself
/// is deliberately leaked: containers with static storage duration are destroyed during exit, and
/// they must still find a valid registry when they fold their statistics.
class SiteRegistry
{
public:
    SiteRecord* resolve(const ContainerStatsSite& site, const ContainerTypeInfo& typeInfo)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        SiteKey key{site.file ? site.file : "<unknown>", site.line, &typeInfo};
        auto found = m_records.find(key);
        if (found != m_records.end())
            return found->second;

        SiteRecord* record = allocateRecord();
        record->file = internString(key.file);
        record->line = site.line;
        record->function = internString(site.function ? site.function : "");
        record->typeInfo = &typeInfo;
        record->sampleRate = 1;

        m_records.emplace(key, record);
        return record;
    }

    /// Returns the record identified by a call stack rather than by a source location, creating it
    /// on first use with `frames` attached as its one backtrace sample.
    ///
    /// String allocations all originate from the same few lines inside the string implementation,
    /// so a source location says nothing about which code is responsible. Keying on the call stack
    /// instead makes the caller the identity of the record.
    SiteRecord* resolveByBacktrace(
        uint64_t stackHash,
        void* const* frames,
        int depth,
        const ContainerTypeInfo& typeInfo,
        uint32_t sampleRate)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        const BacktraceKey key{stackHash, &typeInfo};
        auto found = m_backtraceRecords.find(key);
        if (found != m_backtraceRecords.end())
            return found->second;

        SiteRecord* record = allocateRecord();
        record->file = internString("<backtrace>");
        record->line = 0;
        record->function = internString("");
        record->typeInfo = &typeInfo;
        record->sampleRate = sampleRate;

        const int clamped =
            depth < kContainerStatsBacktraceDepth ? depth : kContainerStatsBacktraceDepth;
        for (int i = 0; i < clamped; ++i)
            record->backtraces[0][i] = frames[i];
        record->backtraceDepths[0] = clamped;
        record->backtraceCount.store(1, std::memory_order_relaxed);

        m_backtraceRecords.emplace(key, record);
        return record;
    }

    template<typename F>
    void forEachRecord(F&& f)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& chunk : m_chunks)
        {
            for (size_t i = 0; i < chunk.used; ++i)
                f(chunk.records[i]);
        }
    }

private:
    static const size_t kChunkSize = 512;

    struct Chunk
    {
        SiteRecord* records;
        size_t used;
    };

    SiteRecord* allocateRecord()
    {
        if (m_chunks.empty() || m_chunks.back().used == kChunkSize)
        {
            Chunk chunk;
            // Value-initialized so that every counter starts at zero.
            chunk.records = new SiteRecord[kChunkSize]();
            chunk.used = 0;
            m_chunks.push_back(chunk);
        }

        Chunk& chunk = m_chunks.back();
        return &chunk.records[chunk.used++];
    }

    /// Returns a pointer to a copy of `text` that lives as long as the registry.
    ///
    /// Site records outlive the per-thread caches and are read during exit, so they cannot borrow
    /// a `std::string` whose storage might be reallocated.
    const char* internString(const std::string& text)
    {
        m_strings.push_back(text);
        return m_strings.back().c_str();
    }

    std::mutex m_mutex;
    std::unordered_map<SiteKey, SiteRecord*, SiteKeyHash> m_records;
    std::unordered_map<BacktraceKey, SiteRecord*, BacktraceKeyHash> m_backtraceRecords;
    std::vector<Chunk> m_chunks;
    /// A deque-like store of interned strings; `std::vector<std::string>` would invalidate the
    /// `c_str()` pointers on reallocation, so a list-of-chunks is used instead.
    std::list<std::string> m_strings;
};

SiteRegistry& getRegistry()
{
    // Intentionally leaked: see the comment on SiteRegistry.
    static SiteRegistry* registry = new SiteRegistry();
    return *registry;
}

/// Writes the report if `SLANG_CONTAINER_STATS` names a path. Registered with `atexit` the first
/// time any site is created.
void writeReportAtExit()
{
    const char* path = getenv("SLANG_CONTAINER_STATS");
    if (path && path[0])
        dumpContainerStats(path);
}

std::once_flag g_atexitRegistered;

} // namespace

ContainerStatsCacheEntry* getContainerStatsCache()
{
    static thread_local ContainerStatsCacheEntry cache[kContainerStatsCacheSize] = {};
    return cache;
}

int containerStatsCaptureBacktrace(void** addresses, int maxDepth)
{
#if SLANG_HAS_BACKTRACE
    return ::backtrace(addresses, maxDepth);
#else
    SLANG_UNUSED(addresses);
    SLANG_UNUSED(maxDepth);
    return 0;
#endif
}

SiteRecord* containerStatsResolveSiteSlow(
    const ContainerStatsSite& site,
    const ContainerTypeInfo& typeInfo)
{
    std::call_once(g_atexitRegistered, []() { atexit(writeReportAtExit); });
    return getRegistry().resolve(site, typeInfo);
}

void ContainerStatsProbe::onConstructed()
{
    if (!m_site)
        return;

    m_site->instances.fetch_add(1, std::memory_order_relaxed);

    const uint64_t live = m_site->live.fetch_add(1, std::memory_order_relaxed) + 1;
    // Deliberately racy: an exact maximum would need a compare-exchange loop on the construction
    // hot path, and this figure only bounds a memory-cost estimate.
    if (live > m_site->liveHighWater.load(std::memory_order_relaxed))
        m_site->liveHighWater.store(live, std::memory_order_relaxed);

    // Sample a backtrace for the first few instances only; capturing costs about a microsecond,
    // which would dominate everything else if it happened per instance.
    const int32_t sampleIndex = m_site->backtraceCount.load(std::memory_order_relaxed);
    if (sampleIndex < kContainerStatsBacktraceSamples)
    {
        const int32_t claimed = m_site->backtraceCount.fetch_add(1, std::memory_order_relaxed);
        if (claimed < kContainerStatsBacktraceSamples)
        {
            m_site->backtraceDepths[claimed] = containerStatsCaptureBacktrace(
                m_site->backtraces[claimed],
                kContainerStatsBacktraceDepth);
        }
    }
}

namespace
{

/// The tag types that name string-buffer allocations in the report.
///
/// String buffers are not a class template, so there is no instantiation to derive a
/// `ContainerTypeInfo` from. These exist purely so that `getContainerTypeInfo` produces one whose
/// signature reads as a recognisable name, letting the offline script treat these records as their
/// own family without a special case in the file format.
///
/// There are two of them because whether a buffer is a string's first or replaces one it already
/// had is a property of the individual allocation, not of the line that made it: the same line in
/// `ensureUniqueStorageWithCapacity` does both. Recording that in the operation mask, which is a
/// union over every event a record ever saw, would mark all of a line's allocations as growths as
/// soon as one of them was. Making it part of the record's identity instead keeps the two counts
/// exact and separately reportable.
struct StringBufferAllocation
{
};
struct StringBufferGrowth
{
};

/// The number of leading stack frames that take part in a string-allocation record's identity.
///
/// A full 16-frame stack would make records proliferate, because two allocations that share an
/// immediate cause but differ deep in the compiler's call tree would land in different records.
/// The frames nearest the allocation are the ones that identify the cause, so only those are
/// hashed; the whole stack is still stored for symbolization.
const int kStringBacktraceKeyDepth = 8;

/// Folds one point event into `record`: a single instance whose lifetime peak was `size`.
///
/// The other containers accumulate into a `ContainerStatsProbe` over an instance's lifetime and
/// fold once at destruction. A string allocation has no such lifetime, so it is folded directly.
void noteEvent(SiteRecord* record, uint64_t size, uint32_t ops)
{
    const int bucket = containerStatsBucketForSize(size);
    record->instances.fetch_add(1, std::memory_order_relaxed);
    record->peakHistogram[bucket].fetch_add(1, std::memory_order_relaxed);
    record->everAllocated.fetch_add(1, std::memory_order_relaxed);
    if (ops)
        record->opMask.fetch_or(ops, std::memory_order_relaxed);
}

} // namespace

void containerStatsNoteStringAllocation(
    const ContainerStatsSite& site,
    uint64_t capacity,
    bool isGrowth)
{
    const ContainerTypeInfo& typeInfo =
        isGrowth ? getContainerTypeInfo<StringBufferGrowth, char, ContainerStatsNoValue>()
                 : getContainerTypeInfo<StringBufferAllocation, char, ContainerStatsNoValue>();

    // The exact, unsampled record, attributed to the line inside the string implementation that
    // asked for the buffer.
    noteEvent(containerStatsResolveSite(site, typeInfo), capacity, 0);

    // Every nth allocation additionally pays for a backtrace, so that the report can also rank the
    // callers responsible rather than only the reasons.
    static thread_local uint32_t sampleCounter = 0;
    if (++sampleCounter < uint32_t(containerStatsStringBacktraceSampleRate))
        return;
    sampleCounter = 0;

    void* frames[kContainerStatsBacktraceDepth];
    const int depth = containerStatsCaptureBacktrace(frames, kContainerStatsBacktraceDepth);
    if (depth <= 0)
        return;

    uint64_t stackHash = 0;
    const int keyDepth = depth < kStringBacktraceKeyDepth ? depth : kStringBacktraceKeyDepth;
    for (int i = 0; i < keyDepth; ++i)
    {
        stackHash ^= uint64_t(reinterpret_cast<uintptr_t>(frames[i]));
        stackHash *= 0x9E3779B97F4A7C15ull;
        stackHash ^= stackHash >> 29;
    }

    SiteRecord* record = getRegistry().resolveByBacktrace(
        stackHash,
        frames,
        depth,
        typeInfo,
        uint32_t(containerStatsStringBacktraceSampleRate));
    noteEvent(record, capacity, 0);
}

void ContainerStatsProbe::fold()
{
    if (!m_site)
        return;

    const int bucket = containerStatsBucketForSize(m_peak);

    m_site->peakHistogram[bucket].fetch_add(1, std::memory_order_relaxed);
    m_site->lookupsByPeak[bucket].fetch_add(m_lookups, std::memory_order_relaxed);
    m_site->insertsByPeak[bucket].fetch_add(m_inserts, std::memory_order_relaxed);
    if (m_ops)
        m_site->opMask.fetch_or(m_ops, std::memory_order_relaxed);
    if (m_peak)
        m_site->everAllocated.fetch_add(1, std::memory_order_relaxed);
    m_site->live.fetch_sub(1, std::memory_order_relaxed);

    m_site = nullptr;
}

namespace
{

/// Appends `text` to `out` with the characters that JSON requires to be escaped replaced.
void appendJsonString(std::string& out, const char* text)
{
    out += '"';
    for (const char* c = text; c && *c; ++c)
    {
        switch (*c)
        {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            if (uint8_t(*c) < 0x20)
            {
                char buf[8];
                snprintf(buf, sizeof(buf), "\\u%04x", *c);
                out += buf;
            }
            else
            {
                out += *c;
            }
            break;
        }
    }
    out += '"';
}

void appendUInt(std::string& out, uint64_t value)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%llu", (unsigned long long)value);
    out += buf;
}

/// Appends one stack frame as `"<module>+0x<offset>"`.
///
/// A raw run-time address is useless to `addr2line`, because a position-independent executable or
/// shared library is loaded at an address chosen at run time. Recording the offset within the
/// module, together with the module's path, is what lets the offline script resolve the frame.
void appendFrame(std::string& out, void* address)
{
#if SLANG_HAS_BACKTRACE
    Dl_info info;
    if (dladdr(address, &info) && info.dli_fname && info.dli_fbase)
    {
        const uintptr_t offset = uintptr_t(address) - uintptr_t(info.dli_fbase);
        char buf[32];
        snprintf(buf, sizeof(buf), "+0x%llx", (unsigned long long)offset);
        out += '"';
        out += info.dli_fname;
        out += buf;
        out += '"';
        return;
    }
#endif
    char buf[32];
    snprintf(buf, sizeof(buf), "\"0x%llx\"", (unsigned long long)(uintptr_t)address);
    out += buf;
}

/// Appends a histogram as a sparse object, because almost every bucket is empty.
void appendHistogram(std::string& out, const std::atomic<uint64_t>* values)
{
    out += '{';
    bool first = true;
    for (int i = 0; i < kContainerStatsBucketCount; ++i)
    {
        const uint64_t v = values[i].load(std::memory_order_relaxed);
        if (!v)
            continue;
        if (!first)
            out += ',';
        first = false;
        out += '"';
        appendUInt(out, uint64_t(i));
        out += "\":";
        appendUInt(out, v);
    }
    out += '}';
}

} // namespace

void dumpContainerStats(const char* path)
{
    if (!path || !path[0])
        return;

    std::string out;
    out.reserve(1 << 20);

    out += "{\n";
    out += "  \"schema\": 1,\n";
    out += "  \"bucketCount\": ";
    appendUInt(out, uint64_t(kContainerStatsBucketCount));
    out += ",\n";
    // Describes how to turn a bucket index back into a size range, so that the offline script does
    // not have to duplicate the bucketing rule.
    out += "  \"bucketRule\": \"index<=32 is an exact size; index>32 covers "
           "(2^(index-28), 2^(index-27)]\",\n";
    out += "  \"records\": [\n";

    bool first = true;
    getRegistry().forEachRecord(
        [&](SiteRecord& record)
        {
            if (!record.instances.load(std::memory_order_relaxed))
                return;

            if (!first)
                out += ",\n";
            first = false;

            out += "    {\"file\": ";
            appendJsonString(out, record.file);
            out += ", \"line\": ";
            appendUInt(out, uint64_t(record.line));
            out += ", \"function\": ";
            appendJsonString(out, record.function);
            out += ", \"signature\": ";
            appendJsonString(out, record.typeInfo->signature);
            out += ", \"keySize\": ";
            appendUInt(out, record.typeInfo->keySize);
            out += ", \"valueSize\": ";
            appendUInt(out, record.typeInfo->valueSize);
            out += ", \"inlineCapacity\": ";
            appendUInt(out, record.typeInfo->inlineCapacity);
            out += ", \"sampleRate\": ";
            appendUInt(out, record.sampleRate ? record.sampleRate : 1);
            out += ", \"keyDefaultConstructible\": ";
            out += record.typeInfo->keyDefaultConstructible ? "true" : "false";
            out += ", \"valueDefaultConstructible\": ";
            out += record.typeInfo->valueDefaultConstructible ? "true" : "false";
            out += ", \"instances\": ";
            appendUInt(out, record.instances.load(std::memory_order_relaxed));
            out += ", \"movedAway\": ";
            appendUInt(out, record.movedAway.load(std::memory_order_relaxed));
            out += ", \"everAllocated\": ";
            appendUInt(out, record.everAllocated.load(std::memory_order_relaxed));
            out += ", \"liveHighWater\": ";
            appendUInt(out, record.liveHighWater.load(std::memory_order_relaxed));
            out += ", \"opMask\": ";
            appendUInt(out, record.opMask.load(std::memory_order_relaxed));
            out += ", \"peakHistogram\": ";
            appendHistogram(out, record.peakHistogram);
            out += ", \"lookupsByPeak\": ";
            appendHistogram(out, record.lookupsByPeak);
            out += ", \"insertsByPeak\": ";
            appendHistogram(out, record.insertsByPeak);

            out += ", \"backtraces\": [";
            const int32_t sampleCount = record.backtraceCount.load(std::memory_order_relaxed);
            const int32_t clamped = sampleCount < kContainerStatsBacktraceSamples
                                        ? sampleCount
                                        : kContainerStatsBacktraceSamples;
            for (int32_t i = 0; i < clamped; ++i)
            {
                if (i)
                    out += ',';
                out += '[';
                for (int32_t d = 0; d < record.backtraceDepths[i]; ++d)
                {
                    if (d)
                        out += ',';
                    appendFrame(out, record.backtraces[i][d]);
                }
                out += ']';
            }
            out += "]}";
        });

    out += "\n  ]\n}\n";

    // The process id is part of the name so that concurrently running compilers, such as the
    // forked `slang-test` servers, each produce their own file for the analysis script to merge.
#if SLANG_WINDOWS_FAMILY
    const int processId = int(_getpid());
#else
    const int processId = int(getpid());
#endif

    std::string fileName = path;
    {
        char buf[32];
        snprintf(buf, sizeof(buf), ".%d.json", processId);
        fileName += buf;
    }

    FILE* file = fopen(fileName.c_str(), "wb");
    if (!file)
        return;
    fwrite(out.data(), 1, out.size(), file);
    fclose(file);
}

} // namespace Slang

#else // SLANG_ENABLE_CONTAINER_STATS

namespace Slang
{
void dumpContainerStats(const char* path)
{
    SLANG_UNUSED(path);
}
} // namespace Slang

#endif // SLANG_ENABLE_CONTAINER_STATS
