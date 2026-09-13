// slang-container-stats.h
#ifndef SLANG_CORE_CONTAINER_STATS_H
#define SLANG_CORE_CONTAINER_STATS_H

// Per-declaration-site statistics for Slang's containers, enabled by the
// `SLANG_ENABLE_CONTAINER_STATS` CMake option.
//
// The goal is to answer, for every place a container is declared in the source, "how big does this
// one actually get, and what operations does it see?", so that declarations which never grow past a
// handful of elements can be converted to the inline-storage `ShortList` / `ShortDictionary`
// variants.
//
// The design constraint that shapes everything here is volume. A single `slangc` invocation
// constructs tens of millions of containers, so nothing may be recorded per instance. Instead each
// construction resolves to a `SiteRecord` -- one per `(file, line, container type)` -- and updates
// counters inside it, and each destruction folds that instance's peak size into the record's
// histogram. Tens of millions of objects therefore collapse into on the order of ten thousand
// records, bounded by the size of the source code rather than by the workload.
//
// When the option is off, every type here becomes an empty struct whose members are no-ops, and
// `SLANG_CONTAINER_STATS_MEMBER` expands to nothing so that no field is added to any container.

#include "slang-common.h"

#include <stdint.h>

#if SLANG_ENABLE_CONTAINER_STATS

#include <atomic>
#include <type_traits>

namespace Slang
{

/// The number of buckets used to summarize a container's peak size.
///
/// Buckets 0 through 32 hold the exact peak size, because the choice between `Dictionary` and
/// `ShortDictionary` turns on small differences in that range. Above 32 the buckets are powers of
/// two, up to 2^20; anything larger is clamped into the last bucket.
static const int kContainerStatsBucketCount = 48;

/// Returns the histogram bucket that summarizes a container whose peak size was `size`.
///
/// Sizes up to 32 map to themselves, so bucket 7 means "peaked at exactly 7 elements". Larger
/// sizes map to one bucket per power of two: 33..64 share a bucket, 65..128 share the next, and so
/// on.
inline int containerStatsBucketForSize(uint64_t size)
{
    if (size <= 32)
        return int(size);

    // Find floor(log2(size - 1)), which is 5 for 33..64, 6 for 65..128, and so on.
    int log2Floor = 0;
    for (uint64_t v = size - 1; v > 1; v >>= 1)
        log2Floor++;

    const int bucket = 33 + (log2Floor - 5);
    return bucket < kContainerStatsBucketCount ? bucket : kContainerStatsBucketCount - 1;
}

/// The operations a container instance was subjected to during its lifetime.
///
/// This exists to disqualify sites rather than to rank them: `ShortDictionary` is add-only, so a
/// site that ever calls `remove` or `set` cannot be converted no matter how small it stays.
enum class ContainerOp : uint32_t
{
    Remove = 1 << 0,
    RemoveIf = 1 << 1,
    Set = 1 << 2,
    Clear = 1 << 3,
    ClearAndDeallocate = 1 << 4,
    Reserve = 1 << 5,
    IndexUpdate = 1 << 6, ///< `operator[]`, which can update an existing value in place.
    CopyAssign = 1 << 7,
    MoveAssign = 1 << 8,
    Swap = 1 << 9,
    Insert = 1 << 10,      ///< Insertion at a position, which `ShortList` supports but is ordered.
    RemoveRange = 1 << 11, ///< Removal of a range or of a single element by index.
    SetCount = 1 << 12,
    /// Iteration. `ShortDictionary` exposes no iterators at all, so a site that iterates cannot be
    /// converted however small it stays.
    Iterate = 1 << 13,
    /// Asking for the element count, which `ShortDictionary` also does not expose.
    GetCount = 1 << 14,
    // Note that there is deliberately no bit here for a `Short*` container spilling out of its
    // inline storage, nor for a string buffer replacing one that already existed. Both of those are
    // properties of an individual instance or event rather than of a declaration, and this mask is
    // a union over everything a record ever saw, so a bit would report "at least one of these
    // promoted" where the question is "how many". A `Short*` promotion is already decided by the
    // data: an instance promoted exactly when its peak exceeded the type's `inlineCapacity`. A
    // string growth is separated by being recorded under its own type tag.
    /// The elements were taken as one contiguous array (`List::getBuffer`, `List::getArrayView`).
    /// `ShortList` splits its elements between an inline array and an overflow buffer, so it can
    /// only satisfy this by allocating and copying; a site that does it cannot be converted.
    ContiguousBuffer = 1 << 17,
    /// Ownership of the buffer was transferred in or out (`List::attachBuffer`/`detachBuffer`),
    /// which `ShortList` cannot express at all.
    AttachBuffer = 1 << 18,
    /// The elements were sorted in place, which `List` does with `std::sort` over its contiguous
    /// buffer and `ShortList` therefore cannot support.
    Sort = 1 << 19,
};

/// Compile-time facts about a container type, recorded once per template instantiation.
///
/// These are what the offline ranking needs in order to decide whether a conversion is even legal
/// (`Short*` requires default-constructible element types) and what it would cost in memory (the
/// inline array is `capacity * (keySize + valueSize)` bytes in every instance).
struct ContainerTypeInfo
{
    /// The compiler's spelling of the enclosing function template, from which the offline script
    /// extracts the container type name. Stored raw so that no parsing happens at runtime.
    const char* signature;
    uint32_t keySize;
    uint32_t valueSize;
    /// The number of elements a `Short*` container holds inline before spilling to the heap, and
    /// zero for a container that has no inline storage.
    ///
    /// This is what turns a `Short*` site's peak histogram into a verdict on its declared
    /// capacity: everything at or below this bound stayed inline, everything above it promoted.
    uint32_t inlineCapacity;
    bool keyDefaultConstructible;
    bool valueDefaultConstructible;
};

/// The value type recorded for a container that stores plain elements rather than key-value pairs.
///
/// `ContainerTypeInfo` carries both a key size and a value size, because the memory an inline
/// array would cost depends on both. A `List` or a `HashSet` has no value type, and recording this
/// one says so explicitly, so that the offline report can charge such a container for its elements
/// alone instead of silently adding the size of whatever stand-in type was passed.
struct ContainerStatsNoValue
{
};

#if SLANG_GCC_FAMILY
#define SLANG_CONTAINER_STATS_SIGNATURE __PRETTY_FUNCTION__
#else
#define SLANG_CONTAINER_STATS_SIGNATURE __FUNCSIG__
#endif

/// Returns the `ContainerTypeInfo` for a container with the given key and value types, and, for a
/// `Short*` container, its inline capacity.
///
/// The returned reference is to a function-local static, so its address is stable and unique per
/// instantiation; that address is what the site cache keys on, which is why this must not return
/// by value.
template<typename TContainer, typename TKey, typename TValue, int kInlineCapacity = 0>
const ContainerTypeInfo& getContainerTypeInfo()
{
    static const ContainerTypeInfo info = {
        SLANG_CONTAINER_STATS_SIGNATURE,
        uint32_t(sizeof(TKey)),
        uint32_t(sizeof(TValue)),
        uint32_t(kInlineCapacity),
        std::is_default_constructible<TKey>::value,
        std::is_default_constructible<TValue>::value,
    };
    return info;
}

/// The source location at which a container was declared.
///
/// The defaults are evaluated at the point of call, so a container constructor that takes this as a
/// defaulted parameter learns its own declaration site with no run-time cost beyond passing three
/// words. A wrapper that embeds a container (such as `HashSetBase`) must take one of these itself
/// and forward it, or the inner container would report the wrapper's constructor instead of the
/// user's declaration.
struct ContainerStatsSite
{
    const char* file = __builtin_FILE();
    int line = __builtin_LINE();
    const char* function = __builtin_FUNCTION();
};

/// The maximum number of raw return addresses captured for a site, symbolized offline.
static const int kContainerStatsBacktraceDepth = 16;

/// The number of instances per site for which a backtrace is captured.
///
/// Capturing costs on the order of a microsecond, so it must not happen per instance; a handful of
/// samples is enough to show which compilation phase a site belongs to.
static const int kContainerStatsBacktraceSamples = 4;

/// The accumulated statistics for one `(file, line, container type)` declaration site.
///
/// Records are created on first use and never freed, and their addresses are stable, so an
/// instance can hold a raw pointer to one for its whole lifetime.
struct SiteRecord
{
    // Identity. The strings are owned by the registry.
    const char* file;
    int line;
    const char* function;
    const ContainerTypeInfo* typeInfo;

    /// Raw return addresses for the first few instances, symbolized offline by `analyze.py`.
    void* backtraces[kContainerStatsBacktraceSamples][kContainerStatsBacktraceDepth];
    std::atomic<int32_t> backtraceCount;
    int32_t backtraceDepths[kContainerStatsBacktraceSamples];

    /// The number of instances constructed at this site.
    std::atomic<uint64_t> instances;
    /// The number of instances whose contents were moved away, and which therefore never folded a
    /// peak into the histogram. `sum(peakHistogram) == instances - movedAway`.
    std::atomic<uint64_t> movedAway;
    /// The number of instances that ever allocated, i.e. whose peak size was non-zero. This is the
    /// numerator of the "heap allocations avoided" benefit estimate.
    std::atomic<uint64_t> everAllocated;
    /// The largest number of instances of this site alive at once, used to bound the memory cost
    /// of giving each instance an inline array. Updated racily, so it is a good estimate rather
    /// than an exact maximum.
    std::atomic<uint64_t> liveHighWater;
    std::atomic<uint64_t> live;

    /// The distribution of per-instance peak sizes.
    std::atomic<uint64_t> peakHistogram[kContainerStatsBucketCount];
    /// Lookups and insertions, binned by the peak bucket of the instance that performed them.
    ///
    /// Binning by peak is what makes the conversion cost computable without keeping per-instance
    /// records: the penalty of an inline array depends on how many lookups happen in instances
    /// that would have stayed inline versus instances that would have been promoted, and that join
    /// is preserved here for any candidate capacity.
    std::atomic<uint64_t> lookupsByPeak[kContainerStatsBucketCount];
    std::atomic<uint64_t> insertsByPeak[kContainerStatsBucketCount];

    /// The union of all operations performed on any instance of this site.
    std::atomic<uint32_t> opMask;

    /// The number of real events each recorded event stands for.
    ///
    /// This is 1 for every record that counts its events exactly, which is all of them except the
    /// backtrace-keyed string-allocation records, where capturing a backtrace per event would be
    /// far too slow and only every nth event is recorded.
    uint32_t sampleRate;
};

/// Returns the record for a site, creating it if this is the first time it has been seen.
///
/// This is the slow path, taken once per site; it takes a lock and keys on string *content* so
/// that a header included into two translation units resolves to a single record.
SiteRecord* containerStatsResolveSiteSlow(
    const ContainerStatsSite& site,
    const ContainerTypeInfo& typeInfo);

/// One entry of the per-thread cache that maps a site to its record.
struct ContainerStatsCacheEntry
{
    const char* file;
    int line;
    const void* typeInfo;
    SiteRecord* record;
};

/// The number of entries in the per-thread site cache. Direct-mapped, so collisions cost a trip to
/// the slow path rather than a wrong answer.
static const int kContainerStatsCacheSize = 4096;

/// Returns this thread's site cache.
ContainerStatsCacheEntry* getContainerStatsCache();

/// Returns the record for a site, which is the operation on the construction hot path.
///
/// `__builtin_FILE()` yields the same string-literal pointer for every construction at a given site
/// within a translation unit, so the common case is a hash, one comparison, and a load -- not a
/// string comparison and not a hash-table probe.
SLANG_FORCE_INLINE SiteRecord* containerStatsResolveSite(
    const ContainerStatsSite& site,
    const ContainerTypeInfo& typeInfo)
{
    uint64_t h = uint64_t(reinterpret_cast<uintptr_t>(site.file));
    h ^= uint64_t(reinterpret_cast<uintptr_t>(&typeInfo)) * 0x9E3779B97F4A7C15ull;
    h ^= uint64_t(uint32_t(site.line)) * 0xBF58476D1CE4E5B9ull;
    h ^= h >> 29;

    ContainerStatsCacheEntry& entry = getContainerStatsCache()[h & (kContainerStatsCacheSize - 1)];
    if (entry.file == site.file && entry.line == site.line && entry.typeInfo == &typeInfo)
        return entry.record;

    SiteRecord* record = containerStatsResolveSiteSlow(site, typeInfo);
    entry.file = site.file;
    entry.line = site.line;
    entry.typeInfo = &typeInfo;
    entry.record = record;
    return record;
}

/// Captures up to `maxDepth` raw return addresses into `addresses`, returning how many were found.
int containerStatsCaptureBacktrace(void** addresses, int maxDepth);

/// Records one allocation of a string buffer able to hold `capacity` characters.
///
/// String buffers need a different shape from the other containers. A `String` is a handle onto a
/// reference-counted, copy-on-write `StringRepresentation`, so the interesting event is not the
/// lifetime of a `String` variable -- copying one allocates nothing -- but each allocation of a
/// representation. An allocation is therefore recorded as a point event: one "instance" whose
/// "peak size" is the capacity allocated. That makes the ordinary report mean the right thing for
/// strings, where the fraction of instances fitting a candidate inline capacity is the fraction of
/// allocations a small-string optimization of that size would remove outright.
///
/// The capacity, rather than the length currently stored, is the quantity recorded, because it is
/// the size an inline buffer would have had to be in order to avoid this allocation. For the paths
/// by which most strings are born -- constructing from a slice -- the two are equal anyway; they
/// differ only where the buffer is deliberately over-allocated so that appending has room to grow.
///
/// `site` names the place inside the string implementation that asked for the buffer, which
/// distinguishes the reasons a buffer is allocated, and `isGrowth` separates reallocating an
/// existing buffer larger from allocating a string's first. Provenance in the caller's own code
/// cannot come from `site`, because every allocation passes through the same handful of lines
/// here; it comes from the sampled backtraces described on
/// `containerStatsStringBacktraceSampleRate`.
void containerStatsNoteStringAllocation(
    const ContainerStatsSite& site,
    uint64_t capacity,
    bool isGrowth);

/// One backtrace is captured for every this many string allocations.
///
/// Attributing string allocations to the code that caused them needs a backtrace, since the
/// declaration site is always inside the string implementation. A backtrace costs on the order of
/// a microsecond, far too much to pay per allocation, so they are sampled; the resulting counts
/// are scaled by this factor offline, which is sound for ranking callers against one another but
/// makes those particular records estimates rather than exact totals.
static const int containerStatsStringBacktraceSampleRate = 64;

/// The per-instance payload that an instrumented container embeds.
///
/// The counters here are deliberately plain integers rather than atomics: they are touched on
/// every lookup and every insertion, and making them atomic would dominate the cost of the
/// instrumentation. They are folded into the shared, atomic `SiteRecord` exactly once, in the
/// destructor. The cost of that choice is that a container shared across threads through a `const`
/// reference can lose counter updates to a race; the totals are therefore approximate, which is
/// acceptable for a ranking.
class ContainerStatsProbe
{
public:
    ContainerStatsProbe(const ContainerStatsSite& site, const ContainerTypeInfo& typeInfo)
        : m_site(containerStatsResolveSite(site, typeInfo))
    {
        onConstructed();
    }

    // Copying or assigning a probe is never right, and leaving the implicit versions in place would
    // let it happen silently. A copied probe would fold a second peak into its site without the
    // matching increment to `instances`, breaking the invariant that the histogram sums to
    // `instances - movedAway` and quietly inflating the numbers the ranking is built on. Deleting
    // them turns that into a compile error in any container that has not declared its own copy and
    // move constructors, which is precisely the set of containers that need to capture a fresh site
    // instead of inheriting one.
    ContainerStatsProbe(const ContainerStatsProbe&) = delete;
    ContainerStatsProbe(ContainerStatsProbe&&) = delete;
    ContainerStatsProbe& operator=(const ContainerStatsProbe&) = delete;
    ContainerStatsProbe& operator=(ContainerStatsProbe&&) = delete;

    /// Takes over the statistics accumulated by `rhs`, which is being moved from.
    ///
    /// The moved-from instance is detached so that it contributes nothing when it is destroyed;
    /// its site counts the event in `movedAway` so that the instance count and the histogram can
    /// still be reconciled.
    void takeFrom(ContainerStatsProbe& rhs)
    {
        if (rhs.m_site)
        {
            m_peak = m_peak > rhs.m_peak ? m_peak : rhs.m_peak;
            m_lookups += rhs.m_lookups;
            m_inserts += rhs.m_inserts;
            m_ops |= rhs.m_ops;

            rhs.m_site->movedAway.fetch_add(1, std::memory_order_relaxed);
            rhs.m_site->live.fetch_sub(1, std::memory_order_relaxed);
            rhs.m_site = nullptr;
            rhs.m_peak = 0;
            rhs.m_lookups = 0;
            rhs.m_inserts = 0;
            rhs.m_ops = 0;
        }
    }

    ~ContainerStatsProbe() { fold(); }

    /// Records that the container now holds `size` elements.
    SLANG_FORCE_INLINE void noteSize(uint64_t size)
    {
        if (size > m_peak)
            m_peak = uint32_t(size);
    }

    SLANG_FORCE_INLINE void noteLookup() { m_lookups++; }
    SLANG_FORCE_INLINE void noteInsert() { m_inserts++; }

    /// Records an insertion into a container that can only ever be added to, whose size is
    /// therefore exactly the number of insertions it has seen.
    ///
    /// `ShortDictionary` is the case this exists for. Its entries live partly in an inline array
    /// and, once it has promoted, entirely in an overflow `Dictionary`, so there is no single
    /// member to read the size from; but it rejects duplicate keys and offers no removal, so
    /// counting insertions gives the size exactly and costs nothing extra.
    SLANG_FORCE_INLINE void noteAddOnlyInsert()
    {
        m_inserts++;
        noteSize(m_inserts);
    }
    SLANG_FORCE_INLINE void noteOp(ContainerOp op) { m_ops |= uint32_t(op); }

private:
    void onConstructed();
    void fold();

    SiteRecord* m_site;
    uint32_t m_peak = 0;
    uint32_t m_lookups = 0;
    uint32_t m_inserts = 0;
    uint32_t m_ops = 0;
};

} // namespace Slang

/// Declares the probe member inside an instrumented container.
///
/// It is `mutable` because lookups are recorded from `const` member functions.
#define SLANG_CONTAINER_STATS_MEMBER mutable ::Slang::ContainerStatsProbe m_containerStatsProbe;

/// Declares the defaulted site parameter that lets a function learn where it was called from.
///
/// This is usually a container's constructor, so that the container learns its own declaration
/// site, but it is also used on the string implementation's allocation helpers so that each one
/// learns which of its callers asked for a buffer.
///
/// The default must be spelled `{}` rather than `ContainerStatsSite()`. With the explicit form the
/// member initializers are evaluated in the context of the struct's own definition, so every site
/// would report this header; with copy-list-initialization they are evaluated at the point of
/// call, which is the whole point.
#define SLANG_CONTAINER_STATS_SITE_PARAM ::Slang::ContainerStatsSite slangContainerStatsSite = {}
#define SLANG_CONTAINER_STATS_SITE_PARAM_TRAILING , SLANG_CONTAINER_STATS_SITE_PARAM

/// Initializes the probe in a constructor's member-initializer list, attributing the container to
/// `site`.
///
/// The `_NEXT` form of each of the macros below is the same thing preceded by a comma, for use
/// when the constructor already initializes other members.
#define SLANG_CONTAINER_STATS_INIT_AT(site, TContainer, TKey, TValue, kCapacity) \
    m_containerStatsProbe(                                                       \
        site,                                                                    \
        ::Slang::getContainerTypeInfo<TContainer, TKey, TValue, kCapacity>())

/// Initializes the probe of a container that has no inline storage.
#define SLANG_CONTAINER_STATS_INIT(TContainer, TKey, TValue) \
    SLANG_CONTAINER_STATS_INIT_AT(slangContainerStatsSite, TContainer, TKey, TValue, 0)
#define SLANG_CONTAINER_STATS_INIT_NEXT(TContainer, TKey, TValue) \
    , SLANG_CONTAINER_STATS_INIT(TContainer, TKey, TValue)
/// The `_ONLY` form supplies the leading colon too, for a constructor whose member-initializer
/// list would otherwise be empty and which must therefore have no colon at all when disabled.
#define SLANG_CONTAINER_STATS_INIT_ONLY(TContainer, TKey, TValue) \
    : SLANG_CONTAINER_STATS_INIT(TContainer, TKey, TValue)

/// Initializes the probe of a `Short*` container, recording its inline capacity so that the
/// offline report can say what fraction of instances outgrew it.
#define SLANG_CONTAINER_STATS_INIT_SHORT(TContainer, TKey, TValue, kCapacity) \
    SLANG_CONTAINER_STATS_INIT_AT(slangContainerStatsSite, TContainer, TKey, TValue, kCapacity)
#define SLANG_CONTAINER_STATS_INIT_SHORT_NEXT(TContainer, TKey, TValue, kCapacity) \
    , SLANG_CONTAINER_STATS_INIT_SHORT(TContainer, TKey, TValue, kCapacity)

/// Initializes the probe of a container whose constructor cannot learn its caller.
///
/// A constructor taking a parameter pack, such as `List(const T& val, Args... args)`, has nowhere
/// to put a defaulted site parameter: anything appended to the signature is swallowed by the pack.
/// Such a constructor attributes its containers to the container's own header instead, so those
/// instances are still counted but are not resolved to the user's declaration.
#define SLANG_CONTAINER_STATS_INIT_UNATTRIBUTED(TContainer, TKey, TValue, kCapacity) \
    SLANG_CONTAINER_STATS_INIT_AT({}, TContainer, TKey, TValue, kCapacity)
#define SLANG_CONTAINER_STATS_INIT_UNATTRIBUTED_NEXT(TContainer, TKey, TValue, kCapacity) \
    , SLANG_CONTAINER_STATS_INIT_UNATTRIBUTED(TContainer, TKey, TValue, kCapacity)
#define SLANG_CONTAINER_STATS_INIT_UNATTRIBUTED_ONLY(TContainer, TKey, TValue, kCapacity) \
    : SLANG_CONTAINER_STATS_INIT_UNATTRIBUTED(TContainer, TKey, TValue, kCapacity)

/// Forwards an already-captured site, for a wrapper that embeds an instrumented container.
#define SLANG_CONTAINER_STATS_FORWARD slangContainerStatsSite
#define SLANG_CONTAINER_STATS_FORWARD_TRAILING , SLANG_CONTAINER_STATS_FORWARD

#define SLANG_CONTAINER_STATS_NOTE_SIZE(size) m_containerStatsProbe.noteSize(size)
#define SLANG_CONTAINER_STATS_NOTE_LOOKUP() m_containerStatsProbe.noteLookup()
#define SLANG_CONTAINER_STATS_NOTE_INSERT() m_containerStatsProbe.noteInsert()
#define SLANG_CONTAINER_STATS_NOTE_ADD_ONLY_INSERT() m_containerStatsProbe.noteAddOnlyInsert()
#define SLANG_CONTAINER_STATS_NOTE_OP(op) m_containerStatsProbe.noteOp(::Slang::ContainerOp::op)
#define SLANG_CONTAINER_STATS_TAKE_FROM(rhs) m_containerStatsProbe.takeFrom(rhs)

/// Declares the defaulted parameters that tell a string-buffer allocation how to attribute itself.
///
/// Three things cannot be worked out from the line that allocates the buffer, so its callers pass
/// them down. Where the buffer was asked for, since every allocation goes through the same few
/// lines. Whether it replaces a buffer the string already had rather than being that string's
/// first, since `ensureUniqueStorageWithCapacity` is where both of those happen. And how many
/// characters actually had to fit, which is not the size of the buffer: that same function rounds
/// its request up to a minimum of 16 and thereafter doubles, so recording the allocated size would
/// put a floor under the histogram and make a small inline buffer look useless when in truth the
/// strings are short. A negative value means the two coincide, which they do everywhere else.
#define SLANG_CONTAINER_STATS_STRING_ALLOC_PARAMS_LEADING                 \
    ::Slang::ContainerStatsSite slangContainerStatsSite = {},             \
                                bool slangContainerStatsIsGrowth = false, \
                                int64_t slangContainerStatsRequiredLength = -1
#define SLANG_CONTAINER_STATS_STRING_ALLOC_PARAMS_TRAILING \
    , SLANG_CONTAINER_STATS_STRING_ALLOC_PARAMS_LEADING

/// Forwards all of them, from one allocation helper to the one it delegates to.
#define SLANG_CONTAINER_STATS_STRING_ALLOC_FORWARD_TRAILING \
    , slangContainerStatsSite, slangContainerStatsIsGrowth, slangContainerStatsRequiredLength

/// Passes an explicit growth flag and required length, from a caller that knows both. The site is
/// spelled `{}` so that it resolves to that caller.
#define SLANG_CONTAINER_STATS_STRING_ALLOC_AT_TRAILING(isGrowth, requiredLength) \
    , {}, isGrowth, int64_t(requiredLength)

/// Passes an explicit growth flag and required length while forwarding the site it was given.
///
/// For a helper that was handed a site to attribute to but works the other two out itself. Using
/// the `AT` form there instead would resolve the site to the helper's own line, so every string
/// that grew would be attributed to the growth code rather than to whoever asked for the string.
#define SLANG_CONTAINER_STATS_STRING_ALLOC_FORWARD_AT_TRAILING(isGrowth, requiredLength) \
    , slangContainerStatsSite, isGrowth, int64_t(requiredLength)

/// Records a string-buffer allocation against the parameters declared above.
#define SLANG_CONTAINER_STATS_NOTE_STRING_ALLOC(capacity)                                    \
    ::Slang::containerStatsNoteStringAllocation(                                             \
        slangContainerStatsSite,                                                             \
        slangContainerStatsRequiredLength < 0 ? uint64_t(capacity)                           \
                                              : uint64_t(slangContainerStatsRequiredLength), \
        slangContainerStatsIsGrowth)

#else // SLANG_ENABLE_CONTAINER_STATS

#define SLANG_CONTAINER_STATS_MEMBER
#define SLANG_CONTAINER_STATS_SITE_PARAM
#define SLANG_CONTAINER_STATS_SITE_PARAM_TRAILING
#define SLANG_CONTAINER_STATS_INIT_AT(site, TContainer, TKey, TValue, kCapacity)
#define SLANG_CONTAINER_STATS_INIT(TContainer, TKey, TValue)
#define SLANG_CONTAINER_STATS_INIT_NEXT(TContainer, TKey, TValue)
#define SLANG_CONTAINER_STATS_INIT_ONLY(TContainer, TKey, TValue)
#define SLANG_CONTAINER_STATS_INIT_SHORT(TContainer, TKey, TValue, kCapacity)
#define SLANG_CONTAINER_STATS_INIT_SHORT_NEXT(TContainer, TKey, TValue, kCapacity)
#define SLANG_CONTAINER_STATS_INIT_UNATTRIBUTED(TContainer, TKey, TValue, kCapacity)
#define SLANG_CONTAINER_STATS_INIT_UNATTRIBUTED_NEXT(TContainer, TKey, TValue, kCapacity)
#define SLANG_CONTAINER_STATS_INIT_UNATTRIBUTED_ONLY(TContainer, TKey, TValue, kCapacity)
#define SLANG_CONTAINER_STATS_FORWARD
#define SLANG_CONTAINER_STATS_FORWARD_TRAILING
#define SLANG_CONTAINER_STATS_NOTE_SIZE(size) \
    do                                        \
    {                                         \
    } while (0)
#define SLANG_CONTAINER_STATS_NOTE_LOOKUP() \
    do                                      \
    {                                       \
    } while (0)
#define SLANG_CONTAINER_STATS_NOTE_INSERT() \
    do                                      \
    {                                       \
    } while (0)
#define SLANG_CONTAINER_STATS_NOTE_ADD_ONLY_INSERT() \
    do                                               \
    {                                                \
    } while (0)
#define SLANG_CONTAINER_STATS_NOTE_OP(op) \
    do                                    \
    {                                     \
    } while (0)
#define SLANG_CONTAINER_STATS_TAKE_FROM(rhs) \
    do                                       \
    {                                        \
    } while (0)
#define SLANG_CONTAINER_STATS_STRING_ALLOC_PARAMS_LEADING
#define SLANG_CONTAINER_STATS_STRING_ALLOC_PARAMS_TRAILING
#define SLANG_CONTAINER_STATS_STRING_ALLOC_FORWARD_TRAILING
#define SLANG_CONTAINER_STATS_STRING_ALLOC_AT_TRAILING(isGrowth, requiredLength)
#define SLANG_CONTAINER_STATS_STRING_ALLOC_FORWARD_AT_TRAILING(isGrowth, requiredLength)
#define SLANG_CONTAINER_STATS_NOTE_STRING_ALLOC(capacity) \
    do                                                    \
    {                                                     \
    } while (0)

#endif // SLANG_ENABLE_CONTAINER_STATS

namespace Slang
{
/// Writes the accumulated container statistics to `path`, as JSON.
///
/// Does nothing unless the build enabled `SLANG_ENABLE_CONTAINER_STATS`. This is also invoked
/// automatically at exit when the `SLANG_CONTAINER_STATS` environment variable names a path, in
/// which case the process id is appended so that concurrently running compilers -- such as the
/// forked `slang-test` servers -- do not overwrite one another.
void dumpContainerStats(const char* path);
} // namespace Slang

#endif // SLANG_CORE_CONTAINER_STATS_H
