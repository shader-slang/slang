// token.cpp
#include "token.h"

#include <assert.h>

namespace Slang {
namespace Compiler {

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

}}
