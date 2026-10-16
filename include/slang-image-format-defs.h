// slang-image-format-defs.h
#ifndef SLANG_FORMAT
    #error Must define SLANG_FORMAT macro before including image-format-defs.h
#endif

// SLANG_FORMAT(NAME, CODE_NAME, DESC)
//
// NAME is the existing image-format token used by reflection, attributes, and
// backend lowering. CODE_NAME is the public Slang type name for the corresponding
// texture format descriptor type.
SLANG_FORMAT(unknown, Unknown, (NONE, 0, 0))
SLANG_FORMAT(rgba32f, RGBA32F, (FLOAT32, 4, sizeof(float) * 4))
SLANG_FORMAT(rgba16f, RGBA16F, (FLOAT16, 4, sizeof(uint16_t) * 4))
SLANG_FORMAT(rg32f, RG32F, (FLOAT32, 2, sizeof(float) * 2))
SLANG_FORMAT(rg16f, RG16F, (FLOAT16, 2, sizeof(uint16_t) * 2))
SLANG_FORMAT(r11f_g11f_b10f, R11G11B10F, (NONE, 3, sizeof(uint32_t)))
SLANG_FORMAT(r32f, R32F, (FLOAT32, 1, sizeof(float)))
SLANG_FORMAT(r16f, R16F, (FLOAT16, 1, sizeof(uint16_t)))
SLANG_FORMAT(rgba16, RGBA16Unorm, (UINT16, 4, sizeof(uint16_t) * 4))
SLANG_FORMAT(rgb10_a2, RGB10A2Unorm, (NONE, 4, sizeof(uint32_t)))
SLANG_FORMAT(rgba8, RGBA8Unorm, (UINT8, 4, sizeof(uint32_t)))
SLANG_FORMAT(rg16, RG16Unorm, (UINT16, 2, sizeof(uint16_t) * 2))
SLANG_FORMAT(rg8, RG8Unorm, (UINT8, 2, sizeof(char) * 2))
SLANG_FORMAT(r16, R16Unorm, (UINT16, 1, sizeof(uint16_t)))
SLANG_FORMAT(r8, R8Unorm, (UINT8, 1, sizeof(uint8_t)))
SLANG_FORMAT(rgba16_snorm, RGBA16Snorm, (UINT16, 4, sizeof(uint16_t) * 4))
SLANG_FORMAT(rgba8_snorm, RGBA8Snorm, (UINT8, 4, sizeof(uint8_t) * 4))
SLANG_FORMAT(rg16_snorm, RG16Snorm, (UINT16, 2, sizeof(uint16_t) * 2))
SLANG_FORMAT(rg8_snorm, RG8Snorm, (UINT8, 2, sizeof(uint8_t) * 2))
SLANG_FORMAT(r16_snorm, R16Snorm, (UINT16, 1, sizeof(uint16_t)))
SLANG_FORMAT(r8_snorm, R8Snorm, (UINT8, 1, sizeof(uint8_t)))
SLANG_FORMAT(rgba32i, RGBA32I, (INT32, 4, sizeof(int32_t) * 4))
SLANG_FORMAT(rgba16i, RGBA16I, (INT16, 4, sizeof(int16_t) * 4))
SLANG_FORMAT(rgba8i, RGBA8I, (INT8, 4, sizeof(int8_t) * 4))
SLANG_FORMAT(rg32i, RG32I, (INT32, 2, sizeof(int32_t) * 2))
SLANG_FORMAT(rg16i, RG16I, (INT16, 2, sizeof(int16_t) * 2))
SLANG_FORMAT(rg8i, RG8I, (INT8, 2, sizeof(int8_t) * 2))
SLANG_FORMAT(r32i, R32I, (INT32, 1, sizeof(int32_t)))
SLANG_FORMAT(r16i, R16I, (INT16, 1, sizeof(int16_t)))
SLANG_FORMAT(r8i, R8I, (INT8, 1, sizeof(int8_t)))
SLANG_FORMAT(rgba32ui, RGBA32UI, (UINT32, 4, sizeof(uint32_t) * 4))
SLANG_FORMAT(rgba16ui, RGBA16UI, (UINT16, 4, sizeof(uint16_t) * 4))
SLANG_FORMAT(rgb10_a2ui, RGB10A2UI, (NONE, 4, sizeof(uint32_t)))
SLANG_FORMAT(rgba8ui, RGBA8UI, (UINT8, 4, sizeof(uint8_t) * 4))
SLANG_FORMAT(rg32ui, RG32UI, (UINT32, 2, sizeof(uint32_t) * 2))
SLANG_FORMAT(rg16ui, RG16UI, (UINT16, 2, sizeof(uint16_t) * 2))
SLANG_FORMAT(rg8ui, RG8UI, (UINT8, 2, sizeof(uint8_t) * 2))
SLANG_FORMAT(r32ui, R32UI, (UINT32, 1, sizeof(uint32_t)))
SLANG_FORMAT(r16ui, R16UI, (UINT16, 1, sizeof(uint16_t)))
SLANG_FORMAT(r8ui, R8UI, (UINT8, 1, sizeof(uint8_t)))
SLANG_FORMAT(r64ui, R64UI, (UINT64, 1, sizeof(uint64_t)))
SLANG_FORMAT(r64i, R64I, (INT64, 1, sizeof(int64_t)))
SLANG_FORMAT(bgra8, BGRA8Unorm, (UINT8, 4, sizeof(uint32_t)))

#undef SLANG_FORMAT
