// slang-mangled-name-parser.cpp
#include "slang-mangled-name-parser.h"

#include <assert.h>

namespace Slang {


UInt MangledNameParser::readCount()
{
    int c = _peek();
    if (!_isDigit((char)c))
    {
        SLANG_UNEXPECTED("bad name mangling");
        UNREACHABLE_RETURN(0);
    }
    _get();

    if (c == '0')
        return 0;

    UInt count = 0;
    for (;;)
    {
        count = count * 10 + c - '0';
        c = _peek();
        if (!_isDigit((char)c))
            return count;

        _get();
    }
}

void MangledNameParser::readGenericParam()
{
    switch (_peek())
    {
    case 'T':
    case 'C':
        _get();
        break;

    case 'v':
        _get();
        readType();
        break;

    default:
        SLANG_UNEXPECTED("bad name mangling");
        break;
    }
}

void MangledNameParser::readGenericParams()
{
    _expect("g");
    UInt paramCount = readCount();
    for (UInt pp = 0; pp < paramCount; pp++)
    {
        readGenericParam();
    }
}

void MangledNameParser::readType()
{
    int c = _peek();
    switch (c)
    {
    case 'V':
    case 'b':
    case 'i':
    case 'u':
    case 'U':
    case 'h':
    case 'f':
    case 'd':
        _get();
        break;

    case 'v':
        _get();
        readSimpleIntVal();
        readType();
        break;

    default:
        readNamedType();
        break;
    }
}

void MangledNameParser::readVal()
{
    switch (_peek())
    {
    case 'k':
        _get();
        readCount();
        break;

    case 'K':
        _get();
        readRawStringSegment();
        break;

    default:
        readType();
        break;
    }

}

void MangledNameParser::readGenericArgs()
{
    _expect("G");
    UInt argCount = readCount();
    for (UInt aa = 0; aa < argCount; aa++)
    {
        readGenericArg();
    }
}

UnownedStringSlice MangledNameParser::readSimpleName()
{
    UnownedStringSlice result;
    for (;;)
    {
        int c = _peek();

        if (c == 'g')
        {
            readGenericParams();
            continue;
        }
        else if (c == 'G')
        {
            readGenericArgs();
            continue;
        }
        else if (c == 'X')
        {
            readExtensionSpec();
            continue;
        }

        if (!_isDigit((char)c))
            return result;

        // Read the length part
        UInt count = readCount();
        if (count > UInt(m_end - m_cursor))
        {
            SLANG_UNEXPECTED("bad name mangling");
            UNREACHABLE_RETURN(result);
        }

        result = UnownedStringSlice(m_cursor, m_cursor + count);
        m_cursor += count;
    }
}

UnownedStringSlice MangledNameParser::readRawStringSegment()
{
    // Read the length part
    UInt count = readCount();
    if (count > UInt(m_end - m_cursor))
    {
        SLANG_UNEXPECTED("bad name mangling");
        UNREACHABLE_RETURN(UnownedStringSlice());
    }

    auto result = UnownedStringSlice(m_cursor, m_cursor + count);
    m_cursor += count;
    return result;
}

UInt MangledNameParser::readParamCount()
{
    _expect("p");
    UInt count = readCount();
    _expect("p");
    return count;
}

} // namespace Slang
