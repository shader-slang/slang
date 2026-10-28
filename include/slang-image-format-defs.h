// clang-format off
#if !defined(SLANG_FORMAT) && !defined(SLANG_FORMAT_WITH_TYPE)
    #error Must define SLANG_FORMAT or SLANG_FORMAT_WITH_TYPE before including image-format-defs.h
#endif

// Preserve the public two-argument SLANG_FORMAT callback. Code that also needs
// the Slang descriptor type name can define SLANG_FORMAT_WITH_TYPE instead.
#ifndef SLANG_FORMAT_WITH_TYPE
    #define SLANG_FORMAT_WITH_TYPE(NAME, CODE_NAME, DESC) SLANG_FORMAT(NAME, DESC)
#endif

SLANG_FORMAT_WITH_TYPE(unknown, Unknown, (NONE, 0, 0))
SLANG_FORMAT_WITH_TYPE(rgba32f, RGBA32F, (FLOAT32, 4, sizeof(float) * 4))
SLANG_FORMAT_WITH_TYPE(rgba16f, RGBA16F, (FLOAT16, 4, sizeof(uint16_t) * 4))
SLANG_FORMAT_WITH_TYPE(rg32f, RG32F, (FLOAT32, 2, sizeof(float) * 2))
SLANG_FORMAT_WITH_TYPE(rg16f, RG16F, (FLOAT16, 2, sizeof(uint16_t) * 2))
SLANG_FORMAT_WITH_TYPE(r11f_g11f_b10f, R11G11B10F, (NONE, 3, sizeof(uint32_t)))
SLANG_FORMAT_WITH_TYPE(r32f, R32F, (FLOAT32, 1, sizeof(float)))
SLANG_FORMAT_WITH_TYPE(r16f, R16F, (FLOAT16, 1, sizeof(uint16_t)))
SLANG_FORMAT_WITH_TYPE(rgba16, RGBA16Unorm, (UINT16, 4, sizeof(uint16_t) * 4))
SLANG_FORMAT_WITH_TYPE(rgb10_a2, RGB10A2Unorm, (NONE, 4, sizeof(uint32_t)))
SLANG_FORMAT_WITH_TYPE(rgba8, RGBA8Unorm, (UINT8, 4, sizeof(uint32_t)))
SLANG_FORMAT_WITH_TYPE(rg16, RG16Unorm, (UINT16, 2, sizeof(uint16_t) * 2))
SLANG_FORMAT_WITH_TYPE(rg8, RG8Unorm, (UINT8, 2, sizeof(char) * 2))
SLANG_FORMAT_WITH_TYPE(r16, R16Unorm, (UINT16, 1, sizeof(uint16_t)))
SLANG_FORMAT_WITH_TYPE(r8, R8Unorm, (UINT8, 1, sizeof(uint8_t)))
SLANG_FORMAT_WITH_TYPE(rgba16_snorm, RGBA16Snorm, (UINT16, 4, sizeof(uint16_t) * 4))
SLANG_FORMAT_WITH_TYPE(rgba8_snorm, RGBA8Snorm, (UINT8, 4, sizeof(uint8_t) * 4))
SLANG_FORMAT_WITH_TYPE(rg16_snorm, RG16Snorm, (UINT16, 2, sizeof(uint16_t) * 2))
SLANG_FORMAT_WITH_TYPE(rg8_snorm, RG8Snorm, (UINT8, 2, sizeof(uint8_t) * 2))
SLANG_FORMAT_WITH_TYPE(r16_snorm, R16Snorm, (UINT16, 1, sizeof(uint16_t)))
SLANG_FORMAT_WITH_TYPE(r8_snorm, R8Snorm, (UINT8, 1, sizeof(uint8_t)))
SLANG_FORMAT_WITH_TYPE(rgba32i, RGBA32I, (INT32, 4, sizeof(int32_t) * 4))
SLANG_FORMAT_WITH_TYPE(rgba16i, RGBA16I, (INT16, 4, sizeof(int16_t) * 4))
SLANG_FORMAT_WITH_TYPE(rgba8i, RGBA8I, (INT8, 4, sizeof(int8_t) * 4))
SLANG_FORMAT_WITH_TYPE(rg32i, RG32I, (INT32, 2, sizeof(int32_t) * 2))
SLANG_FORMAT_WITH_TYPE(rg16i, RG16I, (INT16, 2, sizeof(int16_t) * 2))
SLANG_FORMAT_WITH_TYPE(rg8i, RG8I, (INT8, 2, sizeof(int8_t) * 2))
SLANG_FORMAT_WITH_TYPE(r32i, R32I, (INT32, 1, sizeof(int32_t)))
SLANG_FORMAT_WITH_TYPE(r16i, R16I, (INT16, 1, sizeof(int16_t)))
SLANG_FORMAT_WITH_TYPE(r8i, R8I, (INT8, 1, sizeof(int8_t)))
SLANG_FORMAT_WITH_TYPE(rgba32ui, RGBA32UI, (UINT32, 4, sizeof(uint32_t) * 4))
SLANG_FORMAT_WITH_TYPE(rgba16ui, RGBA16UI, (UINT16, 4, sizeof(uint16_t) * 4))
SLANG_FORMAT_WITH_TYPE(rgb10_a2ui, RGB10A2UI, (NONE, 4, sizeof(uint32_t)))
SLANG_FORMAT_WITH_TYPE(rgba8ui, RGBA8UI, (UINT8, 4, sizeof(uint8_t) * 4))
SLANG_FORMAT_WITH_TYPE(rg32ui, RG32UI, (UINT32, 2, sizeof(uint32_t) * 2))
SLANG_FORMAT_WITH_TYPE(rg16ui, RG16UI, (UINT16, 2, sizeof(uint16_t) * 2))
SLANG_FORMAT_WITH_TYPE(rg8ui, RG8UI, (UINT8, 2, sizeof(uint8_t) * 2))
SLANG_FORMAT_WITH_TYPE(r32ui, R32UI, (UINT32, 1, sizeof(uint32_t)))
SLANG_FORMAT_WITH_TYPE(r16ui, R16UI, (UINT16, 1, sizeof(uint16_t)))
SLANG_FORMAT_WITH_TYPE(r8ui, R8UI, (UINT8, 1, sizeof(uint8_t)))
SLANG_FORMAT_WITH_TYPE(r64ui, R64UI, (UINT64, 1, sizeof(uint64_t)))
SLANG_FORMAT_WITH_TYPE(r64i, R64I, (INT64, 1, sizeof(int64_t)))
SLANG_FORMAT_WITH_TYPE(bgra8, BGRA8Unorm, (UINT8, 4, sizeof(uint32_t)))


#undef SLANG_FORMAT_WITH_TYPE
#undef SLANG_FORMAT
