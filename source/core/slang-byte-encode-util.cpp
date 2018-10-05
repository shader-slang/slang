#include "slang-byte-encode-util.h"



namespace Slang {

// Descriptions of algorithms here...
// https://github.com/stoklund/varint

#if SLANG_LITTLE_ENDIAN && SLANG_UNALIGNED_ACCESS
// Testing on i7, unaligned access is around 40% faster
#   define SLANG_BYTE_ENCODE_USE_UNALIGNED_ACCESS 1
#endif

#ifndef SLANG_BYTE_ENCODE_USE_UNALIGNED_ACCESS
#   define SLANG_BYTE_ENCODE_USE_UNALIGNED_ACCESS 0
#endif

#define SLANG_REPEAT_2(n) n, n
#define SLANG_REPEAT_4(n) SLANG_REPEAT_2(n), SLANG_REPEAT_2(n)
#define SLANG_REPEAT_8(n) SLANG_REPEAT_4(n), SLANG_REPEAT_4(n)
#define SLANG_REPEAT_16(n) SLANG_REPEAT_8(n), SLANG_REPEAT_8(n)
#define SLANG_REPEAT_32(n) SLANG_REPEAT_16(n), SLANG_REPEAT_16(n)
#define SLANG_REPEAT_64(n) SLANG_REPEAT_32(n), SLANG_REPEAT_32(n)
#define SLANG_REPEAT_128(n) SLANG_REPEAT_64(n), SLANG_REPEAT_64(n)

/* static */const int8_t ByteEncodeUtil::s_msb8[256] =
{
    - 1, 
    0, 
    SLANG_REPEAT_2(1), 
    SLANG_REPEAT_4(2), 
    SLANG_REPEAT_8(3), 
    SLANG_REPEAT_16(4), 
    SLANG_REPEAT_32(5),
    SLANG_REPEAT_64(6),
    SLANG_REPEAT_128(7),
};

/* static */size_t ByteEncodeUtil::calcEncodeLiteSizeUInt32(const uint32_t* in, size_t num)
{
    size_t totalNumEncodeBytes = 0;
    
    for (size_t i = 0; i < num; i++)
    {
        const uint32_t v = in[i];

        if (v < kLiteCut1)
        {
            totalNumEncodeBytes += 1;
        }
        else if (v <= kLiteCut1 + 255 * (kLiteCut2 - 1 - kLiteCut1))
        {
            totalNumEncodeBytes += 2;
        }
        else
        {
            totalNumEncodeBytes += calcNonZeroMsByte32(v) + 2;
        }
    }
    return totalNumEncodeBytes;
}

/* static */size_t ByteEncodeUtil::encodeLiteUInt32(const uint32_t* in, size_t num, uint8_t* encodeOut)
{
    uint8_t* encodeStart = encodeOut;

    for (size_t i = 0; i < num; ++i)
    {
        uint32_t v = in[i];

        if(v < kLiteCut1)
        {
            *encodeOut++ = uint8_t(v);
        }
        else if (v <= kLiteCut1 + 255 * (kLiteCut2 - 1 - kLiteCut1))
        {
            v -= kLiteCut1;

            encodeOut[0] = uint8_t(kLiteCut1 + (v >> 8));
            encodeOut[1] = uint8_t(v);
            encodeOut += 2;
        }
        else
        {
            uint8_t* encodeOutStart = encodeOut++;
            while (v)
            {
                *encodeOut++ = uint8_t(v);
                v >>= 8;
            }
            // Finally write the size to the start
            const int numBytes = int(encodeOut - encodeOutStart);
            encodeOutStart[0] = uint8_t(kLiteCut2 + (numBytes - 2));
        }
    }
    return size_t(encodeOut - encodeStart);
}

/* static */void ByteEncodeUtil::encodeLiteUInt32(const uint32_t* in, size_t num, List<uint8_t>& encodeArrayOut)
{
    // Make sure there is at least enough space for all bytes
    encodeArrayOut.SetSize(num);

    uint8_t* encodeOut = encodeArrayOut.begin();
    uint8_t* encodeOutEnd = encodeArrayOut.end();

    for (size_t i = 0; i < num; ++i)
    {
        // Check if we need some more space
        if (encodeOut + kMaxLiteEncodeUInt32 > encodeOutEnd)
        {
            const size_t offset = size_t(encodeOut - encodeArrayOut.begin());

            const UInt oldCapacity = encodeArrayOut.Capacity();
           
            // Make some more space
            encodeArrayOut.Reserve(oldCapacity + (oldCapacity >> 1) + kMaxLiteEncodeUInt32);
            // Make the size the capacity
            const UInt capacity = encodeArrayOut.Capacity();
            encodeArrayOut.SetSize(capacity);

            encodeOut = encodeArrayOut.begin() + offset;
            encodeOutEnd = encodeArrayOut.end();
        }

        uint32_t v = in[i];

        if (v < kLiteCut1)
        {
            *encodeOut++ = uint8_t(v);
        }
        else if (v <= kLiteCut1 + 255 * (kLiteCut2 - 1 - kLiteCut1))
        {
            v -= kLiteCut1;

            encodeOut[0] = uint8_t(kLiteCut1 + (v >> 8));
            encodeOut[1] = uint8_t(v);
            encodeOut += 2;
        }
        else
        {
            uint8_t* encodeOutStart = encodeOut++;
            while (v)
            {
                *encodeOut++ = uint8_t(v);
                v >>= 8;
            }
            // Finally write the size to the start
            const int numBytes = int(encodeOut - encodeOutStart);
            encodeOutStart[0] = uint8_t(kLiteCut2 + (numBytes - 2));
        }
    }

    encodeArrayOut.SetSize(UInt(encodeOut - encodeArrayOut.begin()));
    encodeArrayOut.Compress();
}

/* static */int ByteEncodeUtil::encodeLiteUInt32(uint32_t in, uint8_t out[kMaxLiteEncodeUInt32])
{
    // 0-184        1 byte    value = B0
    // 185 - 248    2 bytes   value = 185 + 256 * (B0 - 185) + B1
    // 249 - 255    3 - 9 bytes value = (B0 - 249 + 2) little - endian bytes following B0.

    if (in < kLiteCut1)
    {
        out[0] = uint8_t(in);
        return 1;
    }
    else if (in <= kLiteCut1 + 255 * (kLiteCut2 - 1 - kLiteCut1))
    {
        in -= kLiteCut1;

        out[0] = uint8_t(kLiteCut1 + (in >> 8));
        out[1] = uint8_t(in);
        return 2;
    }
    else
    {
        int numBytes = 1;
        while (in)
        {
            out[numBytes++] = uint8_t(in);
            in >>= 8;
        }
        // Finally write the size
        out[0] = uint8_t(kLiteCut2 + (numBytes - 2));
        return numBytes;
    }
}

static const uint32_t s_unalignedUInt32Mask[5] = 
{
    0x00000000,
    0x000000ff,
    0x0000ffff,
    0x00ffffff,
    0xffffffff,
};

/* static */int ByteEncodeUtil::decodeLiteUInt32(const uint8_t* in, uint32_t* out)
{
    uint8_t b0 = *in++;
    if (b0 < kLiteCut1)
    {
        *out = uint32_t(b0);
        return 1;
    }
    else if (b0 < kLiteCut2)
    {
        uint8_t b1 = *in++;
        *out = kLiteCut1 + b1 + (uint32_t(b0 - kLiteCut1) << 8);
        return 2;
    }
    else
    {
        int numBytesRemaining = b0 - kLiteCut2 + 2 - 1;
        
#if SLANG_BYTE_ENCODE_USE_UNALIGNED_ACCESS
        //const uint32_t mask = s_unalignedUInt32Mask[numBytesRemaining];
        const uint32_t mask = ~(uint32_t(0xffffff00) << ((numBytesRemaining - 1) * 8));
        const uint32_t value = (*(const uint32_t*)in) & mask;
#else
        // This works on all cpus although slower
        uint32_t value = in[0];

        switch (numBytesRemaining)
        {
            case 4: value |= uint32_t(in[3]) << 24;         /* fall thru */
            case 3: value |= uint32_t(in[2]) << 16;         /* fall thru */
            case 2: value |= uint32_t(in[1]) << 8;          /* fall thru */
            case 1: break;
        }        
#endif
        *out = value;
        return numBytesRemaining + 1;
    }
}

/* static */size_t ByteEncodeUtil::decodeLiteUInt32(const uint8_t* encodeIn, size_t numValues, uint32_t* valuesOut)
{
    const uint8_t* encodeStart = encodeIn;

    for (size_t i = 0; i < numValues; ++i)
    {
        uint8_t b0 = *encodeIn++;
        if (b0 < kLiteCut1)
        {
            valuesOut[i] = uint32_t(b0);
        }
        else if (b0 < kLiteCut2)
        {
            uint8_t b1 = *encodeIn++;
            valuesOut[i] = kLiteCut1 + b1 + (uint32_t(b0 - kLiteCut1) << 8);
        }
        else
        {
            int numBytesRemaining = b0 - kLiteCut2 + 2 - 1;

#if SLANG_BYTE_ENCODE_USE_UNALIGNED_ACCESS
            const uint32_t mask = s_unalignedUInt32Mask[numBytesRemaining];
            //const uint32_t mask = ~(uint32_t(0xffffff00) << ((numBytesRemaining - 1) * 8));
            const uint32_t value = (*(const uint32_t*)encodeIn) & mask;
#else
            // This works on all cpus although slower
            uint32_t value = encodeIn[0];
            switch (numBytesRemaining)
            {
                case 4: value |= uint32_t(encodeIn[3]) << 24;         /* fall thru */
                case 3: value |= uint32_t(encodeIn[2]) << 16;         /* fall thru */
                case 2: value |= uint32_t(encodeIn[1]) << 8;          /* fall thru */
                case 1: break;
            }
#endif  
            valuesOut[i] = value;
            encodeIn += numBytesRemaining;
        }
    }

    return size_t(encodeIn - encodeStart);
}

} // namespace Slang
