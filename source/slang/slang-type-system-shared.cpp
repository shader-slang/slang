#include "slang-type-system-shared.h"

#include "../core/slang-common.h"

namespace Slang
{
    TextureFlavor TextureFlavor::create(SlangResourceShape shape, SlangResourceAccess access)
    {
        TextureFlavor rs;
        rs.flavor = uint16_t(shape | (access << 8));
        return rs;
    }

    TextureFlavor TextureFlavor::create(SlangResourceShape shape, SlangResourceAccess access, int flags)
    {
        SLANG_ASSERT((flags & ~int(SLANG_RESOURCE_EXT_SHAPE_MASK)) == 0);
        TextureFlavor rs;
        rs.flavor = uint16_t(shape | (access << 8) | flags);
        return rs;
    }
}
