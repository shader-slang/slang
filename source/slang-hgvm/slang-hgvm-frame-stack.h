#pragma once

#include "core/slang-basic.h"
#include "slang-hgvm-execution-mode.h"

#include <memory>

namespace Slang::HGVM
{

/// Allocates interpreter frames at host addresses that never move while the frame is live.
///
/// Consider `accumulate(inout uint total, uint depth)` recursing 2000 levels: every level stores
/// the raw host address of the caller's `total`. Because a pushed frame is never relocated, that
/// address stays valid until the frame that owns `total` is popped, so the VM can pass frame
/// addresses to callees (and later to native code) without translating them.
///
/// Frames are pushed and popped in LIFO order. `push` takes a fast inline path while the frame
/// fits the current chunk of storage and otherwise asks the implementation for more storage.
/// When the configured byte limit would be exceeded, `push` returns null and the caller reports a
/// stack overflow; the stack never grows without bound.
class FrameStack
{
public:
    virtual ~FrameStack() = default;

    /// Reserves `byteSize` bytes aligned to `alignment` and returns their stable address, or null
    /// when the stack limit would be exceeded. The bytes are not initialized.
    SLANG_FORCE_INLINE uint8_t* push(size_t byteSize, size_t alignment)
    {
        SLANG_ASSERT(alignment && !(alignment & (alignment - 1)));
        uintptr_t start = (uintptr_t(m_top) + alignment - 1) & ~uintptr_t(alignment - 1);
        if (start >= uintptr_t(m_top) && start <= uintptr_t(m_end) &&
            byteSize <= uintptr_t(m_end) - start)
        {
            m_top = reinterpret_cast<uint8_t*>(start + byteSize);
            return reinterpret_cast<uint8_t*>(start);
        }
        return pushSlow(byteSize, alignment);
    }

    /// Returns the current top, which a later `popTo` restores.
    uint8_t* getTop() const { return m_top; }

    /// Pops every frame pushed after `getTop()` returned `top`.
    SLANG_FORCE_INLINE void popTo(uint8_t* top)
    {
        if (top >= m_base && top <= m_top)
            m_top = top;
        else
            popToSlow(top);
    }

    /// Returns whether `[address, address + byteSize)` lies entirely inside live frame storage.
    /// The debugger uses this to accept raw frame addresses without exposing other host memory.
    virtual bool containsLiveRange(const void* address, size_t byteSize) const = 0;

    /// Returns the maximum number of bytes this stack may hold before `push` reports overflow.
    size_t getByteLimit() const { return m_byteLimit; }

protected:
    explicit FrameStack(size_t byteLimit)
        : m_byteLimit(byteLimit)
    {
    }

    /// Provides storage for a frame that does not fit between `m_top` and `m_end`.
    virtual uint8_t* pushSlow(size_t byteSize, size_t alignment) = 0;

    /// Restores a top that lies outside the chunk currently described by `m_base`..`m_top`.
    virtual void popToSlow(uint8_t* top) = 0;

    /// Start of the chunk that contains `m_top`; tops in `[m_base, m_top]` pop on the fast path.
    uint8_t* m_base = nullptr;
    uint8_t* m_top = nullptr;
    /// End of the storage that `push` may use without calling `pushSlow`.
    uint8_t* m_end = nullptr;
    size_t m_byteLimit = 0;
};

/// Creates a frame stack of the requested kind. `segmentByteSize` applies to segmented stacks
/// only and is the size of each newly allocated segment unless a single frame needs more.
std::unique_ptr<FrameStack> createFrameStack(
    FrameStackKind kind,
    size_t byteLimit,
    size_t segmentByteSize);

} // namespace Slang::HGVM
