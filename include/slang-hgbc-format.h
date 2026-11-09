// slang-hgbc-format.h
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace Slang
{

using HGBCId = uint32_t;

constexpr uint32_t makeHGBCFourCC(char a, char b, char c, char d)
{
    return (uint32_t(uint8_t(a)) << 0) | (uint32_t(uint8_t(b)) << 8) |
           (uint32_t(uint8_t(c)) << 16) | (uint32_t(uint8_t(d)) << 24);
}

static constexpr uint32_t kHGBCMagic = makeHGBCFourCC('H', 'G', 'B', 'C');

// The Lua schema is the source of truth for every wire enum, opcode, and operand grammar.
#if 0 // FIDDLE TEMPLATE:
% require("source/slang/slang-hgbc-opcodes.lua").emit_cpp()
#else // FIDDLE OUTPUT:
#define FIDDLE_GENERATED_OUTPUT_ID 0
#include "slang-hgbc-format.h.fiddle"
#endif // FIDDLE END

/// Reports whether a format contains a depth aspect. Clear validation and pipeline signatures
/// use the same classification; a shader's floating element type alone does not imply depth.
inline bool isHGBCDepthFormat(HGBCResourceFormat format)
{
    return format == HGBCResourceFormat::D32Float || format == HGBCResourceFormat::D16Unorm ||
           format == HGBCResourceFormat::D32FloatS8Uint;
}

/// Returns whether an HGBC texture format contains a stencil aspect.
inline bool hasHGBCStencilAspect(HGBCResourceFormat format)
{
    return format == HGBCResourceFormat::D32FloatS8Uint;
}

struct HGBCInstHeader
{
    uint16_t opcode;
    uint8_t extension;
    uint8_t operandCount;
};

static_assert(sizeof(HGBCInstHeader) == 4, "HGBC instruction headers must be 4 bytes");

} // namespace Slang
