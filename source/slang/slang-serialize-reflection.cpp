// slang-serialize-reflection.cpp
#include "slang-serialize-reflection.h"

#include "slang-serialize.h"

namespace Slang {

bool ReflectClassInfo::isSubClassOfSlow(const ThisType& super) const
{
    ReflectClassInfo const* info = this;
    while (info)
    {
        if (info == &super)
            return true;
        info = info->m_superClass;
    }
    return false;
}


} // namespace Slang
