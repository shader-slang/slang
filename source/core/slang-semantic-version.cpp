// slang-semantic-version.cpp
#include "slang-semantic-version.h"

#include "../../slang-com-helper.h"

#include "../core/slang-string-util.h"

namespace Slang {

SlangResult SemanticVersion::parse(const UnownedStringSlice& value, SemanticVersion& outVersion)
{
    outVersion.reset();

    UnownedStringSlice slices[3];
    const Index splitCount = StringUtil::split(value, '.', 3, slices);
    if (!(splitCount > 0 && splitCount <= 3 && slices[splitCount - 1].end() == value.end()))
    {
        return SLANG_FAIL;
    }

    Int ints[3] = { 0, 0, 0 };
    for (Index i = 0; i < splitCount; i++)
    {
        SLANG_RETURN_ON_FAIL(StringUtil::parseInt(slices[i], ints[i]));

        const Int max = (i == 0) ? 0xffffffff : 0xffff;
        if (ints[i] < 0 || ints[i] > max)
        {
            return SLANG_FAIL;
        }
    }

    outVersion.m_major = uint32_t(ints[0]);
    outVersion.m_minor = uint16_t(ints[1]);
    outVersion.m_patch = uint16_t(ints[2]);

    return SLANG_OK;
}

void SemanticVersion::append(StringBuilder& buf) const
{
    buf << Int32(m_major) << "." << Int32(m_minor);
    if (m_patch != 0)
    {
        buf << "." << Int32(m_patch);
    }
}

} // namespace Slang
