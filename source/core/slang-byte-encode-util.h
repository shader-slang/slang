#ifndef SLANG_BYTE_ENCODE_UTIL_H
#define SLANG_BYTE_ENCODE_UTIL_H

#include "list.h"


namespace Slang {

struct ByteEncodeUtil
{
    enum
    {
        kMaxLiteEncodeUInt32 = 5,                   /// One byte for prefix, the remaining 4 bytes hold the value
    };

        /** Encodes a uint32_t as an integer
         @return the number of bytes needed to encode */
    static int encodeLiteUInt32(uint32_t in, uint8_t out[kMaxLiteEncodeUInt32]);

        /** Decode a lite encoding. 
         @param in The lite encoded bytes 
         @param out Value constructed
        @return number of bytes on in consumed */
    static int decodeLiteUInt32(const uint8_t* in, uint32_t* out);
};

} // namespace Slang

#endif // SLANG_BYTE_ENCODE_UTIL_H
