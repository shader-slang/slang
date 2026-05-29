#ifndef SLANG_TARGET_TYPE_LAYOUT_INFO_H
#define SLANG_TARGET_TYPE_LAYOUT_INFO_H

#include <cstdint>

namespace Slang
{

struct TargetBuiltinTypeLayoutInfo
{
    uint32_t genericPointerSize;
    uint32_t stringSize, stringAlignment;
};

} // namespace Slang

#endif
