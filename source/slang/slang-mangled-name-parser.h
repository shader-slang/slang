// slang-mangled-name-parser.h
#ifndef SLANG_MANGLED_NAME_PARSER_H_INCLUDED
#define SLANG_MANGLED_NAME_PARSER_H_INCLUDED

#include "../core/basic.h"

#include "compiler.h"

namespace Slang
{

class MangledNameParser
{
public:
    
        // Call at the beginning of a mangled name,
        // to strip off the main prefix
    void startUnmangling() { _expect("_S"); }

    UInt readCount();

    void readGenericParam();

    void readGenericParams();

    SLANG_INLINE void readSimpleIntVal();

    UnownedStringSlice readRawStringSegment();

    void readNamedType();

    void readType();

    void readVal();

    void readGenericArg() { readVal(); }

    void readGenericArgs();

    SLANG_INLINE void readExtensionSpec();

    UnownedStringSlice readSimpleName();

    UInt readParamCount();

        /// Ctor
    SLANG_FORCE_INLINE MangledNameParser(String const& str);

private:
    char const* m_cursor = nullptr;
    char const* m_begin = nullptr;
    char const* m_end = nullptr;

    static bool _isDigit(char c) { return (c >= '0') && (c <= '9'); }

    char _peek() { return *m_cursor; }
        // TODO(JS): This could perhaps do with a better name, as 'get' makes it appear as if it has no effect. So '_next' would
        // perhaps be clearer
    char _get() { return *m_cursor++; }

    void _expect(char c)
    {
        if (_peek() == c)
        {
            _get();
        }
        else
        {
            // ERROR!
            SLANG_UNEXPECTED("mangled name error");
        }
    }

    void _expect(char const* str)
    {
        while (char c = *str++)
            _expect(c);
    }

};

// -------------------------------------------------------------------------- -
SLANG_FORCE_INLINE MangledNameParser::MangledNameParser(String const& str)
    : m_cursor(str.begin())
    , m_begin(str.begin())
    , m_end(str.end())
{}

// ---------------------------------------------------------------------------
SLANG_INLINE void MangledNameParser::readSimpleIntVal()
{
    int c = _peek();
    if (_isDigit((char)c))
    {
        _get();
    }
    else
    {
        readVal();
    }
}

// ---------------------------------------------------------------------------
SLANG_INLINE void MangledNameParser::readNamedType()
{
    // TODO: handle types with more complicated names
    readRawStringSegment();
}

// ---------------------------------------------------------------------------
SLANG_INLINE void MangledNameParser::readExtensionSpec()
{
    _expect("X");
    readType();
}

}
#endif
