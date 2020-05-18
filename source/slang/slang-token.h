// slang-token.h
#ifndef SLANG_TOKEN_H_INCLUDED
#define SLANG_TOKEN_H_INCLUDED

#include "../core/slang-basic.h"

#include "slang-source-loc.h"
#include "slang-name.h"

namespace Slang {

class Name;

enum class TokenType : uint8_t
{
#define TOKEN(NAME, DESC) NAME,
#include "slang-token-defs.h"
};

char const* TokenTypeToString(TokenType type);

typedef uint8_t TokenFlags;
struct TokenFlag
{
    enum Enum : TokenFlags
    {
        AtStartOfLine           = 1 << 0,
        AfterWhitespace         = 1 << 1,
        SuppressMacroExpansion  = 1 << 2,
        ScrubbingNeeded         = 1 << 3,
        Name                    = 1 << 4,           ///< If set the ptr points to the name
    };
};

class Token
{
public:

    TokenType   type = TokenType::Unknown;
    TokenFlags  flags = 0;

    SourceLoc   loc;
    uint32_t charsCount;              ///< Only valid if the chars is valid (ie is not a name)

    union CharsNameUnion
    {
        const char* chars;
        Name* name;
    };

    CharsNameUnion charsNameUnion;

    UnownedStringSlice getContent() const;
        /// Set content
    void setContent(const UnownedStringSlice& content);

    Name* getName() const;

    Name* getNameOrNull() const;

    SourceLoc getLoc() const { return loc; }

    SLANG_FORCE_INLINE void setName(Name* inName) { flags |= TokenFlag::Name; charsNameUnion.name = inName; }

    Token():
        charsCount(0)
    {
        charsNameUnion.chars = nullptr;
    }

    Token(
        TokenType typeIn,
        const UnownedStringSlice & contentIn,
        SourceLoc locIn,
        TokenFlags flagsIn = 0)
        : flags(flagsIn)
	{
        SLANG_ASSERT((flagsIn & TokenFlag::Name) == 0); 
		type = typeIn;
        charsNameUnion.chars = contentIn.begin();
        charsCount = uint32_t(contentIn.getLength());
        loc = locIn;
	}
};

// ---------------------------------------------------------------------------
SLANG_FORCE_INLINE UnownedStringSlice Token::getContent() const
{
    return (flags & TokenFlag::Name) ? charsNameUnion.name->text.getUnownedSlice() : UnownedStringSlice(charsNameUnion.chars, charsCount);
}

// ---------------------------------------------------------------------------
SLANG_FORCE_INLINE Name* Token::getName() const
{
    return getNameOrNull();
}

// ---------------------------------------------------------------------------
SLANG_FORCE_INLINE Name* Token::getNameOrNull() const
{
    return (flags & TokenFlag::Name) ? charsNameUnion.name : nullptr;
}

// ---------------------------------------------------------------------------
SLANG_FORCE_INLINE void Token::setContent(const UnownedStringSlice& content)
{
    flags &= ~TokenFlag::Name;
    charsNameUnion.chars = content.begin();
    charsCount = uint32_t(content.getLength());
}



} // namespace Slang

#endif
