// slang-mangled-lexer.cpp
#include "slang-mangled-lexer.h"

#include <assert.h>

namespace Slang {

UInt MangledLexer::readCount()
{
    int c = _peek();
    if (!_isDigit((char)c))
    {
        SLANG_UNEXPECTED("bad name mangling");
        UNREACHABLE_RETURN(0);
    }
    _next();

    if (c == '0')
        return 0;

    UInt count = 0;
    for (;;)
    {
        count = count * 10 + c - '0';
        c = _peek();
        if (!_isDigit((char)c))
            return count;

        _next();
    }
}

void MangledLexer::readGenericParam()
{
    switch (_peek())
    {
    case 'T':
    case 'C':
        _next();
        break;

    case 'v':
        _next();
        readType();
        break;

    default:
        SLANG_UNEXPECTED("bad name mangling");
        break;
    }
}

void MangledLexer::readGenericParams()
{
    _expect("g");
    UInt paramCount = readCount();
    for (UInt pp = 0; pp < paramCount; pp++)
    {
        readGenericParam();
    }
}

void MangledLexer::readType()
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
        _next();
        break;

    case 'v':
        _next();
        readSimpleIntVal();
        readType();
        break;

    default:
        readNamedType();
        break;
    }
}

void MangledLexer::readVal()
{
    switch (_peek())
    {
    case 'k':
        _next();
        readCount();
        break;

    case 'K':
        _next();
        readRawStringSegment();
        break;

    default:
        readType();
        break;
    }

}

void MangledLexer::readGenericArgs()
{
    _expect("G");
    UInt argCount = readCount();
    for (UInt aa = 0; aa < argCount; aa++)
    {
        readGenericArg();
    }
}

UnownedStringSlice MangledLexer::readSimpleName()
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

UnownedStringSlice MangledLexer::readRawStringSegment()
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

UInt MangledLexer::readParamCount()
{
    _expect("p");
    UInt count = readCount();
    _expect("p");
    return count;
}

} // namespace Slang
