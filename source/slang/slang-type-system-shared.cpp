#include "slang-type-system-shared.h"

namespace Slang
{
    TextureFlavor TextureFlavor::create(SlangResourceShape shape, SlangResourceAccess access)
    {
        TextureFlavor rs;
        rs.flavor = uint16_t(shape | (access << 8));
        return rs;
    }
}
