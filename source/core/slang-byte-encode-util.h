#ifndef SLANG_CORE_BYTE_ENCODE_UTIL_H
#define SLANG_CORE_BYTE_ENCODE_UTIL_H

#include "slang-common.h"

namespace Slang
{

struct ByteEncodeUtil
{
    /** Find the most significant bit for 8 bits
    @param v The value to find most significant bit on
    @return The most significant bit, or -1 if no bits are set
    */
    SLANG_FORCE_INLINE static int calcMsb8(uint32_t v);

    /** Find the most significant bit for 32 bits
    @param v The value to find most significant bit on
    @return The most significant bit, or -1 if no bits are set
    */
    SLANG_FORCE_INLINE static int calcMsb32(uint32_t v);

    /// Table that maps 8 bits to it's most significant bit. If 0 returns -1.
    static const int8_t s_msb8[256];
};

#if SLANG_VC
// Works on ARM and x86/64 on visual studio compiler

// ---------------------------------------------------------------------------
SLANG_FORCE_INLINE /* static */ int ByteEncodeUtil::calcMsb8(uint32_t v)
{
    SLANG_ASSERT((v & 0xffffff00) == 0);
    if (v == 0)
    {
        return -1;
    }
    unsigned long index;
    _BitScanReverse(&index, v);
    return index;
}

// ---------------------------------------------------------------------------
SLANG_FORCE_INLINE /* static */ int ByteEncodeUtil::calcMsb32(uint32_t v)
{
    if (v == 0)
    {
        return -1;
    }
    unsigned long index;
    _BitScanReverse(&index, v);
    return index;
}

#else

// ---------------------------------------------------------------------------
SLANG_FORCE_INLINE /* static */ int ByteEncodeUtil::calcMsb8(uint32_t v)
{
    SLANG_ASSERT((v & 0xffffff00) == 0);
    return s_msb8[v];
}

// ---------------------------------------------------------------------------
SLANG_FORCE_INLINE /* static */ int ByteEncodeUtil::calcMsb32(uint32_t v)
{
    return (v & 0xffff0000) ? ((v & 0xff000000) ? s_msb8[v >> 24] + 24 : s_msb8[v >> 16] + 16)
                            : ((v & 0x0000ff00) ? s_msb8[v >> 8] + 8 : s_msb8[v]);
}

#endif

} // namespace Slang

#endif // SLANG_BYTE_ENCODE_UTIL_H
