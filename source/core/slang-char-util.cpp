#include "slang-char-util.h"

namespace Slang {

static const CharUtil::CharFlagMap _calcCharFlagsMap()
{
    typedef CharUtil::Flag Flag;

    CharUtil::CharFlagMap map;
    memset(&map, 0, sizeof(map));

    {
        for (Index i = 'a'; i <= 'z'; ++i)
        {
            map.flags[i] |= Flag::Lower;
        }
    }
    {
        for (Index i = 'A'; i <= 'Z'; ++i)
        {
            map.flags[i] |= Flag::Upper;
        }
    }
    {
        for (Index i = '0'; i <= '9'; ++i)
        {
            map.flags[i] |= Flag::Digit | Flag::HexDigit;
        }
    }
    {
        for (Index i = 'a'; i <= 'f'; ++i)
        {
            map.flags[i] |= Flag::HexDigit;
            map.flags[size_t(CharUtil::toUpper(char(i)))] |= Flag::HexDigit;
        }
    }

    {
        map.flags[size_t(' ')] |= Flag::HorizontalWhitespace;
        map.flags[size_t('\t')] |= Flag::HorizontalWhitespace;
    }

    return map;
}

/* static */const CharUtil::CharFlagMap CharUtil::g_charFlagMap = _calcCharFlagsMap();

} // namespace Slang
