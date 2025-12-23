#ifndef SLANG_TARGET_TYPE_LAYOUT_INFO_H
#define SLANG_TARGET_TYPE_LAYOUT_INFO_H

#include "slang.h"
#include "slang-type-system-shared.h"

namespace Slang
{

// Interface for querying layout information of target-specific builtin types.
struct ITargetBuiltinTypeLayoutInfo : public ISlangUnknown
{
    SLANG_COM_INTERFACE(
        0x9e27b35e,
        0xba2c,
        0x4f59,
        {0xb5, 0xf4, 0xdc, 0x93, 0x6d, 0xde, 0xaf, 0xe7});

    // Must return false if the pointer cannot be known.
    virtual SLANG_NO_THROW bool SLANG_MCALL getPointerSize(
        uint32_t& size,
        uint32_t& preferredAlignment,
        AddressSpace addressSpace = AddressSpace::Generic) = 0;

    // Must return false if the resource size cannot be known.
    virtual SLANG_NO_THROW bool SLANG_MCALL getResourceSize(
        uint32_t& size,
        uint32_t& preferredAlignment,
        SlangResourceShape resource) = 0;
};

} // namespace Slang

#endif
