// token.cpp
#include "token.h"

#include <assert.h>

namespace Slang {

char const* TokenTypeToString(TokenType type)
{
    switch( type )
    {
    default:
        assert(!"unexpected");
        return "<uknown>";

#define TOKEN(NAME, DESC) case TokenType::NAME: return DESC;
#include "token-defs.h"
    }
}

} // namespace Slang
