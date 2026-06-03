// slang-offset-container.cpp
#include "slang-offset-container.h"

namespace Slang
{

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! OffsetString !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

size_t OffsetString::calcEncodedSize(size_t size, uint8_t encode[kMaxSizeEncodeSize])
{
    SLANG_ASSERT(size <= 0xffffffff);
    if (size <= kSizeBase)
    {
        encode[0] = uint8_t(size);
        return 1;
    }
    // Encode
    int num = 0;
    while (size)
    {
        encode[num + 1] = uint8_t(size);
        size >>= 8;
        num++;
    }

    // It might be one byte past the front, if its < 0x100 but greater than kSizeBase
    SLANG_ASSERT(num >= 1);

    encode[0] = uint8_t(kSizeBase + num);
    return num + 1;
}

/* static */ const char* OffsetString::decodeSize(const char* in, size_t& outSize)
{
    const uint8_t* cur = (const uint8_t*)in;
    if (*cur <= kSizeBase)
    {
        outSize = *cur;
        return in + 1;
    }

    int numBytes = *cur - kSizeBase;
    switch (numBytes)
    {
    case 1:
        {
            outSize = cur[1];
            return in + 2;
        }
    case 2:
        {
            outSize = cur[1] | (uint32_t(cur[2]) << 8);
            return in + 3;
        }
    case 3:
        {
            outSize = cur[1] | (uint32_t(cur[2]) << 8) | (uint32_t(cur[3]) << 16);
            return in + 4;
        }
    case 4:
        {
            outSize = cur[1] | (uint32_t(cur[2]) << 8) | (uint32_t(cur[3]) << 16) |
                      (uint32_t(cur[4]) << 24);
            return in + 5;
        }
    default:
        {
            outSize = 0;
            return nullptr;
        }
    }
}

/* static */ size_t OffsetString::calcAllocationSize(size_t stringSize)
{
    uint8_t encode[kMaxSizeEncodeSize];
    size_t encodeSize = calcEncodedSize(stringSize, encode);
    // Add 1 for terminating 0
    return encodeSize + stringSize + 1;
}

/* static */ size_t OffsetString::calcAllocationSize(const UnownedStringSlice& slice)
{
    return calcAllocationSize(slice.getLength());
}

UnownedStringSlice OffsetString::getSlice() const
{
    size_t size;
    const char* chars = decodeSize(m_sizeThenContents, size);

    return UnownedStringSlice(chars, size);
}

const char* OffsetString::getCstr() const
{
    return getSlice().begin();
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! OffsetContainer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

OffsetContainer::OffsetContainer()
{
    static_assert(
        kStartOffset > 0 && kStartOffset <= kMax32Offset,
        "kStartOffset must fit in the 32-bit offset domain");

    m_capacity = 0;
    m_data = nullptr;

    // We need to allocate some of the first bytes so that offset 0 can be used for
    // kNull32Offset. If this 8-byte allocation fails the container has no way to maintain
    // that invariant — a subsequent newObject() would hand out offset 0 and collide with
    // the null sentinel. Treat this as an unrecoverable startup failure.
    void* start = allocateAndZero(kStartOffset, 1);
    SLANG_RELEASE_ASSERT(start != nullptr);
}

OffsetContainer::~OffsetContainer()
{
    if (m_data)
    {
        ::free(m_data);
    }
}

void* OffsetContainer::allocate(size_t size)
{
    return allocate(size, 1);
}

void OffsetContainer::fixAlignment(size_t alignment)
{
    allocate(0, alignment);
}

void* OffsetContainer::allocate(size_t size, size_t alignment)
{
    // Alignment must be a non-zero power of two for the bitwise alignment math below.
    if (alignment == 0 || (alignment & (alignment - 1)) != 0)
    {
        return nullptr;
    }

    // The container addresses memory via 32-bit offsets (Offset32Ptr), so reject any
    // allocation that would grow the data beyond the 32-bit addressable range. Using the
    // 32-bit limit (rather than SIZE_MAX) is tighter than required for size_t arithmetic,
    // but prevents silent truncation when offsets are converted to uint32_t.
    const size_t kMaxDataSize = size_t(kMax32Offset);
    const size_t alignmentMask = alignment - 1;
    // On 64-bit hosts a caller can request an alignment larger than the 32-bit offset
    // domain (e.g., 2^33). That would make alignmentMask > kMaxDataSize and cause the
    // subtraction in the next guard to underflow, silently bypassing the check.
    if (alignmentMask > kMaxDataSize)
    {
        return nullptr;
    }
    if (m_dataSize > kMaxDataSize - alignmentMask)
    {
        return nullptr;
    }

    size_t offset = (m_dataSize + alignmentMask) & ~alignmentMask;
    if (size > kMaxDataSize - offset)
    {
        return nullptr;
    }

    const size_t minSize = offset + size;

    if (minSize > m_capacity)
    {
        size_t calcSize = m_capacity;
        if (calcSize < 2048)
        {
            calcSize = 2048;
        }
        else
        {
            // Expand geometrically, but lets not double in size...
            calcSize = calcSize + (calcSize / 2);
        }

        // We must be at least minSize
        size_t newSize = (calcSize < minSize) ? minSize : calcSize;
        if (newSize > kMaxDataSize)
        {
            newSize = kMaxDataSize;
        }

        // Reallocate space
        uint8_t* newData = (uint8_t*)::realloc(m_data, newSize);
        if (!newData)
        {
            return nullptr;
        }
        m_data = newData;
        m_capacity = newSize;
    }

    SLANG_ASSERT(minSize <= m_capacity);

    m_dataSize = minSize;
    return m_data + offset;
}

void* OffsetContainer::allocateAndZero(size_t size, size_t alignment)
{
    void* data = allocate(size, alignment);
    if (!data)
    {
        return nullptr;
    }
    memset(data, 0, size);
    return data;
}

Offset32Ptr<OffsetString> OffsetContainer::newString(const UnownedStringSlice& slice)
{
    size_t stringSize = slice.getLength();

    // OffsetString encodes the size in at most kMaxSizeEncodeSize bytes: a 1-byte header
    // followed by up to 4 bytes of little-endian size data.
    if (stringSize > size_t(kMax32Offset) - OffsetString::kMaxSizeEncodeSize - 1)
    {
        return Offset32Ptr<OffsetString>();
    }

    uint8_t head[OffsetString::kMaxSizeEncodeSize];
    size_t headSize = OffsetString::calcEncodedSize(stringSize, head);

    size_t allocSize = headSize + stringSize + 1;
    uint8_t* bytes = (uint8_t*)allocate(allocSize);
    if (!bytes)
    {
        return Offset32Ptr<OffsetString>();
    }

    ::memcpy(bytes, head, headSize);
    ::memcpy(bytes + headSize, slice.begin(), stringSize);

    // 0 terminate
    bytes[headSize + stringSize] = 0;

    return Offset32Ptr<OffsetString>(getOffset(bytes));
}

Offset32Ptr<OffsetString> OffsetContainer::newString(const char* contents)
{
    Offset32Ptr<OffsetString> relString;
    if (contents)
    {
        relString = newString(UnownedStringSlice(contents));
    }
    return relString;
}

} // namespace Slang
