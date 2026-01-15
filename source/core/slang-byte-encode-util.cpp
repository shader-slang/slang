#include "slang-byte-encode-util.h"

namespace Slang
{

// Descriptions of algorithms here...
// https://github.com/stoklund/varint

#define SLANG_REPEAT_2(n) n, n
#define SLANG_REPEAT_4(n) SLANG_REPEAT_2(n), SLANG_REPEAT_2(n)
#define SLANG_REPEAT_8(n) SLANG_REPEAT_4(n), SLANG_REPEAT_4(n)
#define SLANG_REPEAT_16(n) SLANG_REPEAT_8(n), SLANG_REPEAT_8(n)
#define SLANG_REPEAT_32(n) SLANG_REPEAT_16(n), SLANG_REPEAT_16(n)
#define SLANG_REPEAT_64(n) SLANG_REPEAT_32(n), SLANG_REPEAT_32(n)
#define SLANG_REPEAT_128(n) SLANG_REPEAT_64(n), SLANG_REPEAT_64(n)

/* static */ const int8_t ByteEncodeUtil::s_msb8[256] = {
    -1,
    0,
    SLANG_REPEAT_2(1),
    SLANG_REPEAT_4(2),
    SLANG_REPEAT_8(3),
    SLANG_REPEAT_16(4),
    SLANG_REPEAT_32(5),
    SLANG_REPEAT_64(6),
    SLANG_REPEAT_128(7),
};

} // namespace Slang
