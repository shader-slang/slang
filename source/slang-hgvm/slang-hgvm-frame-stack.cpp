#include "slang-hgvm-frame-stack.h"

#include <new>

#if SLANG_WINDOWS_FAMILY
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#elif !SLANG_WASM
#include <sys/mman.h>
#endif

namespace Slang::HGVM
{
namespace
{

/// Frame alignment never exceeds the staged storage alignment, so segment bases use the same.
static constexpr size_t kSegmentAlignment = 64;

/// Reserved stacks commit memory in chunks of this size as the stack top advances.
static constexpr size_t kCommitGranularity = 64 * 1024;

/// Rounds `value` up to a multiple of the power-of-two `alignment`, or returns zero on overflow.
size_t alignUp(size_t value, size_t alignment)
{
    size_t mask = alignment - 1;
    if (value > SIZE_MAX - mask)
        return 0;
    return (value + mask) & ~mask;
}

#if !SLANG_WASM
/// Reserves one contiguous address range up front and commits pages as frames reach them.
///
/// Consider a 64 MiB limit: construction reserves 64 MiB of address space but commits none of
/// it. The first `push` commits one 64 KiB chunk; later pushes commit further chunks only when
/// the stack top crosses into them. Because the range never moves, every frame address stays
/// valid until the frame is popped, and a push past the reserved range reports overflow.
class ReservedFrameStack : public FrameStack
{
public:
    explicit ReservedFrameStack(size_t byteLimit)
        : FrameStack(alignUp(byteLimit, kCommitGranularity))
    {
        if (!m_byteLimit)
            throw Exception("HGVM frame stack limit is too large");
#if SLANG_WINDOWS_FAMILY
        void* base = VirtualAlloc(nullptr, m_byteLimit, MEM_RESERVE, PAGE_NOACCESS);
#else
        void* base = mmap(
            nullptr,
            m_byteLimit,
            PROT_NONE,
            MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE,
            -1,
            0);
        if (base == MAP_FAILED)
            base = nullptr;
#endif
        if (!base)
            throw Exception("HGVM failed to reserve address space for its frame stack");
        m_reservedBase = static_cast<uint8_t*>(base);
        m_base = m_reservedBase;
        m_top = m_reservedBase;
        m_end = m_reservedBase;
    }

    ~ReservedFrameStack() override
    {
#if SLANG_WINDOWS_FAMILY
        VirtualFree(m_reservedBase, 0, MEM_RELEASE);
#else
        munmap(m_reservedBase, m_byteLimit);
#endif
    }

    bool containsLiveRange(const void* address, size_t byteSize) const override
    {
        auto start = static_cast<const uint8_t*>(address);
        return start >= m_reservedBase && start <= m_top && byteSize <= size_t(m_top - start);
    }

protected:
    /// Commits enough additional pages for the frame, or reports overflow at the reserve limit.
    uint8_t* pushSlow(size_t byteSize, size_t alignment) override
    {
        size_t start = alignUp(size_t(m_top - m_reservedBase), alignment);
        if (!start && m_top != m_reservedBase)
            return nullptr;
        if (start > m_byteLimit || byteSize > m_byteLimit - start)
            return nullptr;
        size_t requiredEnd = start + byteSize;
        size_t committedEnd = size_t(m_end - m_reservedBase);
        size_t newEnd = alignUp(requiredEnd, kCommitGranularity);
        if (!newEnd || newEnd > m_byteLimit)
            newEnd = m_byteLimit;
        SLANG_ASSERT(newEnd > committedEnd);
#if SLANG_WINDOWS_FAMILY
        if (!VirtualAlloc(
                m_reservedBase + committedEnd,
                newEnd - committedEnd,
                MEM_COMMIT,
                PAGE_READWRITE))
            throw Exception("HGVM failed to commit frame stack memory");
#else
        if (mprotect(
                m_reservedBase + committedEnd,
                newEnd - committedEnd,
                PROT_READ | PROT_WRITE) != 0)
            throw Exception("HGVM failed to commit frame stack memory");
#endif
        m_end = m_reservedBase + newEnd;
        m_top = m_reservedBase + requiredEnd;
        return m_reservedBase + start;
    }

    /// All tops of a reserved stack lie in its single chunk, so only invalid tops arrive here.
    void popToSlow(uint8_t* top) override
    {
        SLANG_UNUSED(top);
        SLANG_RELEASE_ASSERT(!"HGVM popped its frame stack to a top that was never pushed");
    }

private:
    uint8_t* m_reservedBase = nullptr;
};
#endif

/// Allocates frames from a chain of separately allocated segments that never move.
///
/// Consider a 4 KiB segment size and a recursion whose frames are 48 bytes: the first 85 frames
/// fill segment zero, and the next push starts segment one instead of moving segment zero. A
/// pointer into segment zero therefore stays valid while deeper frames live in later segments.
/// A popped segment is kept for reuse so a recursion that repeatedly crosses a segment boundary
/// does not allocate on every call. The WebAssembly build uses this form because it cannot
/// reserve address space without committing it.
class SegmentedFrameStack : public FrameStack
{
public:
    SegmentedFrameStack(size_t byteLimit, size_t segmentByteSize)
        : FrameStack(byteLimit), m_segmentByteSize(alignUp(segmentByteSize, kSegmentAlignment))
    {
        SLANG_RELEASE_ASSERT(m_segmentByteSize);
        // The first segment exists from the start, so every top returned by `getTop` (including
        // the initial one) lies in some segment and `popTo` can always find it.
        Segment segment;
        segment.size = m_segmentByteSize < byteLimit ? m_segmentByteSize : byteLimit;
        segment.base = static_cast<uint8_t*>(
            ::operator new(segment.size, std::align_val_t(kSegmentAlignment)));
        m_segments.add(segment);
        m_current = 0;
        m_base = segment.base;
        m_top = segment.base;
        m_end = segment.base + segment.size;
    }

    ~SegmentedFrameStack() override
    {
        for (const Segment& segment : m_segments)
            ::operator delete(segment.base, std::align_val_t(kSegmentAlignment));
    }

    bool containsLiveRange(const void* address, size_t byteSize) const override
    {
        auto start = static_cast<const uint8_t*>(address);
        for (Index i = 0; i <= m_current; ++i)
        {
            const Segment& segment = m_segments[i];
            uint8_t* liveEnd = i == m_current ? m_top : segment.liveEnd;
            if (start >= segment.base && start <= liveEnd && byteSize <= size_t(liveEnd - start))
                return true;
        }
        return false;
    }

protected:
    /// Moves to the next segment, reusing a cached one when the frame fits and allocating
    /// otherwise; returns null when the segments in use would exceed the byte limit.
    uint8_t* pushSlow(size_t byteSize, size_t alignment) override
    {
        m_segments[m_current].liveEnd = m_top;
        Index next = m_current + 1;
        if (next < m_segments.getCount() && m_segments[next].size < byteSize)
        {
            // A cached segment that is too small for this frame cannot serve it or any later
            // order of pushes better than a fresh segment would, so the stale tail is released.
            for (Index i = next; i < m_segments.getCount(); ++i)
                ::operator delete(m_segments[i].base, std::align_val_t(kSegmentAlignment));
            m_segments.setCount(next);
        }
        if (next == m_segments.getCount())
        {
            size_t size = byteSize > m_segmentByteSize ? alignUp(byteSize, kSegmentAlignment)
                                                       : m_segmentByteSize;
            if (!size || getBytesInUse(next) > m_byteLimit ||
                size > m_byteLimit - getBytesInUse(next))
            {
                return nullptr;
            }
            Segment segment;
            segment.base =
                static_cast<uint8_t*>(::operator new(size, std::align_val_t(kSegmentAlignment)));
            segment.size = size;
            m_segments.add(segment);
        }
        else if (getBytesInUse(next + 1) > m_byteLimit)
        {
            return nullptr;
        }
        SLANG_ASSERT(alignment <= kSegmentAlignment);
        SLANG_UNUSED(alignment);
        m_current = next;
        Segment& segment = m_segments[next];
        m_base = segment.base;
        m_end = segment.base + segment.size;
        m_top = segment.base + byteSize;
        return segment.base;
    }

    /// Returns to the earlier segment that contains `top`.
    void popToSlow(uint8_t* top) override
    {
        for (Index i = m_current - 1; i >= 0; --i)
        {
            Segment& segment = m_segments[i];
            if (top >= segment.base && top <= segment.liveEnd)
            {
                m_current = i;
                m_base = segment.base;
                m_end = segment.base + segment.size;
                m_top = top;
                return;
            }
        }
        SLANG_RELEASE_ASSERT(!"HGVM popped its frame stack to a top that was never pushed");
    }

private:
    struct Segment
    {
        uint8_t* base = nullptr;
        size_t size = 0;
        /// The top this segment had when a push moved to the next segment.
        uint8_t* liveEnd = nullptr;
    };

    /// Returns the total size of the first `count` segments.
    size_t getBytesInUse(Index count) const
    {
        size_t result = 0;
        for (Index i = 0; i < count && i < m_segments.getCount(); ++i)
            result += m_segments[i].size;
        return result;
    }

    size_t m_segmentByteSize = 0;
    List<Segment> m_segments;
    Index m_current = 0;
};

} // namespace

std::unique_ptr<FrameStack> createFrameStack(
    FrameStackKind kind,
    size_t byteLimit,
    size_t segmentByteSize)
{
    SLANG_RELEASE_ASSERT(byteLimit);
    if (kind == FrameStackKind::Default)
        kind = SLANG_WASM ? FrameStackKind::Segmented : FrameStackKind::Reserved;
#if !SLANG_WASM
    if (kind == FrameStackKind::Reserved)
        return std::make_unique<ReservedFrameStack>(byteLimit);
#endif
    return std::make_unique<SegmentedFrameStack>(byteLimit, segmentByteSize);
}

} // namespace Slang::HGVM
