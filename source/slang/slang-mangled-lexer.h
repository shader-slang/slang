// slang-mangled-lexer.h
#ifndef SLANG_MANGLED_LEXER_H_INCLUDED
#define SLANG_MANGLED_LEXER_H_INCLUDED

#include "../core/slang-basic.h"

#include "slang-compiler.h"

namespace Slang
{

/* A lexer like utility class used for decoding mangled names.
Expects names to be correctly constructed - any errors will cause asserts/failures */
class MangledLexer
{
public:
        /// Reads a count at current position 
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
    SLANG_FORCE_INLINE MangledLexer(const UnownedStringSlice& slice);

private:

    // Call at the beginning of a mangled name,
    // to strip off the main prefix
    void _start() { _expect("_S"); }

    static bool _isDigit(char c) { return (c >= '0') && (c <= '9'); }

        /// Returns the character at the current position
    char _peek() { return *m_cursor; }
        // Returns the current character and moves to next character.
    char _next() { return *m_cursor++; }

    SLANG_INLINE void _expect(char c);

    void _expect(char const* str)
    {
        while (char c = *str++)
            _expect(c);
    }

    char const* m_cursor = nullptr;
    char const* m_begin = nullptr;
    char const* m_end = nullptr;
};

// -------------------------------------------------------------------------- -
SLANG_FORCE_INLINE MangledLexer::MangledLexer(const UnownedStringSlice& slice)
    : m_cursor(slice.begin())
    , m_begin(slice.begin())
    , m_end(slice.end())
{
    _start();
}

// ---------------------------------------------------------------------------
SLANG_INLINE void MangledLexer::readSimpleIntVal()
{
    int c = _peek();
    if (_isDigit((char)c))
    {
        _next();
    }
    else
    {
        readVal();
    }
}

// ---------------------------------------------------------------------------
SLANG_INLINE void MangledLexer::readNamedType()
{
    // TODO: handle types with more complicated names
    readRawStringSegment();
}

// ---------------------------------------------------------------------------
SLANG_INLINE void MangledLexer::readExtensionSpec()
{
    _expect("X");
    readType();
}

// ---------------------------------------------------------------------------
SLANG_INLINE void MangledLexer::_expect(char c)
{
    if (_peek() == c)
    {
        _next();
    }
    else
    {
        // ERROR!
        SLANG_UNEXPECTED("mangled name error");
    }
}

}
#endif
