// Runtime consistency check for the platform macros slang.h derives from the toolchain: confirms
// SLANG_PTR_IS_* / SLANG_*_ENDIAN match the pointer size and byte order this build actually uses,
// so a wrong derivation cannot silently corrupt the SlangInt/SlangUInt ABI. Complements the
// header's own pointer-size static_assert and its endianness #error guards.

#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdint.h>
#include <string.h>

SLANG_UNIT_TEST(platformPointerSize)
{
    SLANG_CHECK((SLANG_PTR_IS_64 ? 8u : 4u) == sizeof(void*));
    SLANG_CHECK(SLANG_PTR_IS_32 == (SLANG_PTR_IS_64 ? 0 : 1));
    SLANG_CHECK(sizeof(SlangInt) == sizeof(void*));
    SLANG_CHECK(sizeof(SlangUInt) == sizeof(void*));
    SLANG_CHECK(sizeof(SlangSizeT) == sizeof(void*));
}

SLANG_UNIT_TEST(platformEndianness)
{
    SLANG_CHECK((SLANG_LITTLE_ENDIAN ^ SLANG_BIG_ENDIAN) == 1);

    const uint32_t value = 0x01020304u;
    unsigned char bytes[sizeof(value)];
    memcpy(bytes, &value, sizeof(value));

    const bool runtimeIsLittleEndian = (bytes[0] == 0x04);
    SLANG_CHECK(runtimeIsLittleEndian == (SLANG_LITTLE_ENDIAN != 0));
    SLANG_CHECK(runtimeIsLittleEndian != (SLANG_BIG_ENDIAN != 0));
}
