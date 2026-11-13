//TEST:SIMPLE: -Itests/preprocessor

// Adjacent angle brackets select header-name lexing before the operand is read.
#include<header-name//regular.slang.h>

// Spaces and comment-like slashes are characters in a physical header name.
#include<header-name/header with spaces.slang.h>

// An expanded angle-bracket operand is assembled from ordinary macro tokens.
#define HEADER_OPERAND <header-name/regular.slang.h>
#include HEADER_OPERAND

#define HEADER_WITH_SPACES <header-name/header with spaces.slang.h>
#include HEADER_WITH_SPACES

#define QUOTED_HEADER "header-name/regular.slang.h"
#include QUOTED_HEADER

#if HEADER_NAME_REGULAR != 17
#error The regular header was not included.
#endif

#if HEADER_NAME_WITH_SPACES != 23
#error The header containing spaces was not included.
#endif

int headerNameResult()
{
    return HEADER_NAME_REGULAR + HEADER_NAME_WITH_SPACES;
}
