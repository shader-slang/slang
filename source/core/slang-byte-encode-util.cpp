#include "slang-byte-encode-util.h"

namespace Slang {

enum 
{
    kLiteCut1 = 185,
    kLiteCut2 = 249,

};

/* static */int ByteEncodeUtil::encodeLiteUInt32(uint32_t in, uint8_t out[kMaxLiteEncodeUInt32])
{
    // 0-184   1 byte    value = B0
    // 185 - 248 2 bytes   value = 185 + 256 * (B0 - 185) + B1
    // 249 - 255 3 - 9 bytes value = (B0 - 249 + 2) little - endian bytes following B0.

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
        
        // I could do an unaligned load here and mask, but just do something 
        // dumb for now.

        uint32_t value = in[0];

        switch (numBytesRemaining)
        {
            case 4: value |= uint32_t(in[3]) << 24;         /* fall thru */
            case 3: value |= uint32_t(in[2]) << 16;         /* fall thru */
            case 2: value |= uint32_t(in[1]) << 8;          /* fall thru */
            case 1: break;
        }

        *out = value;
        return numBytesRemaining + 1;
    }
}

} // namespace Slang
