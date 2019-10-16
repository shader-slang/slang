// slang-relative-containere.cpp
#include "slang-relative-container.h"

namespace Slang {

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! RelativeString !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

size_t RelativeString::calcEncodedSize(size_t size, uint8_t encode[kMaxSizeEncodeSize])
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

/* static */const char* RelativeString::decodeSize(const char* in, size_t& outSize)
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
            outSize = cur[1] | (uint32_t(cur[2]) << 8) | (uint32_t(cur[3]) << 16) | (uint32_t(cur[4]) << 24);
            return in + 5;
        }
        default:
        {
            outSize = 0;
            return nullptr;
        }
    }
}

/* static */size_t RelativeString::calcAllocationSize(size_t stringSize)
{
    uint8_t encode[kMaxSizeEncodeSize];
    size_t encodeSize = calcEncodedSize(stringSize, encode);
    // Add 1 for terminating 0
    return encodeSize + stringSize + 1;
}

/* static */size_t RelativeString::calcAllocationSize(const UnownedStringSlice& slice)
{
    return calcAllocationSize(slice.size());
}

UnownedStringSlice RelativeString::getSlice() const
{
    size_t size;
    const char* chars = decodeSize(m_sizeThenContents, size);

    return UnownedStringSlice(chars, size);
}

const char* RelativeString::getCstr() const
{
    return getSlice().begin();
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! RelativeContainer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

RelativeContainer::RelativeContainer()
{
    m_current = 0;
}

void* RelativeContainer::allocate(size_t size)
{
    return allocate(size, 1);
}

void* RelativeContainer::allocate(size_t size, size_t alignment)
{
    size_t offset = (m_current + alignment - 1) & ~(alignment - 1);

    size_t dataSize = m_data.getCount();

    if (offset + size > dataSize)
    {
        const size_t minSize = offset + size;

        size_t calcSize = dataSize;
        if (dataSize < 2048)
        {
            calcSize = 2048;
        }
        else
        {
            // Expand geometrically, but lets not double in size...
            calcSize = dataSize + (dataSize / 2);
        }

        // We must be at least calc size
        size_t newSize = (calcSize < minSize) ? minSize : calcSize;
        m_data.setCount(newSize);

        //m_data.getCapacity();

        dataSize = newSize;
    }
    SLANG_ASSERT(offset + size <= dataSize);

    m_current = offset + size;

    return m_data.getBuffer() + offset;
}

void* RelativeContainer::allocateAndZero(size_t size, size_t alignment)
{
    void* data = allocate(size, alignment);
    memset(data, 0, size);
    return data;
}

Safe32Ptr<RelativeString> RelativeContainer::newString(const UnownedStringSlice& slice)
{
    size_t stringSize = slice.size();

    size_t allocSize = RelativeString::calcAllocationSize(stringSize);
    uint8_t* bytes = (uint8_t*)allocate(allocSize);

    size_t headSize = RelativeString::calcEncodedSize(slice.size(), bytes);

    ::memcpy(bytes + headSize, slice.begin(), stringSize);

    // 0 terminate
    bytes[headSize + stringSize] = 0;

    return Safe32Ptr<RelativeString>(getOffset(bytes), this);
}

Safe32Ptr<RelativeString> RelativeContainer::newString(const char* contents)
{
    Safe32Ptr<RelativeString> relString;
    if (contents)
    {
        relString = newString(UnownedStringSlice(contents));
    }
    return relString;
}


} // namespace Slang
