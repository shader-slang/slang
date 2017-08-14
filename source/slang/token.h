// token.h
#ifndef SLANG_TOKEN_H_INCLUDED
#define SLANG_TOKEN_H_INCLUDED

#include "../core/basic.h"

#include "source-loc.h"

namespace Slang {

class Name;

enum class TokenType
{
#define TOKEN(NAME, DESC) NAME,
#include "token-defs.h"
};

char const* TokenTypeToString(TokenType type);

enum TokenFlag : unsigned int
{
    AtStartOfLine           = 1 << 0,
    AfterWhitespace         = 1 << 1,
    SuppressMacroExpansion  = 1 << 2,
    ScrubbingNeeded         = 1 << 3,
};
typedef unsigned int TokenFlags;

class Token
{
public:
    TokenType   type = TokenType::Unknown;
    TokenFlags  flags = 0;

    SourceLoc   loc;
    void*       ptrValue;

    String Content;

    Token() = default;

    Token(
        TokenType type,
        const String & content,
        SourceLoc loc,
        TokenFlags flags = 0)
        : flags(flags)
	{
		type = type;
		Content = content;
        loc = loc;
        ptrValue = nullptr;
	}

    Name* getName() const;

    Name* getNameOrNull() const;

    SourceLoc getLoc() const { return loc; }
};



} // namespace Slang

#endif
