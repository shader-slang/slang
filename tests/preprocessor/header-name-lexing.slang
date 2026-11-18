//TEST:SIMPLE: -Itests/preprocessor

// We compile include operands in both physical and macro-expanded forms.
// Filenames with spaces expose missing separators, and ordinary filenames expose added ones.

// Adjacent angle brackets select header-name lexing before the operand is read.
#include<header-name//regular.slang.h>

// Spaces and comment-like slashes are characters in a physical header name.
#include<header-name/header with spaces.slang.h>

// A comment or escaped newline before the operand must not end header-name lexing.
#include/**/<header-name//regular.slang.h>
#include\
<header-name/header with spaces.slang.h>

// An expanded angle-bracket operand is assembled from ordinary macro tokens.
#define HEADER_OPERAND <header-name/regular.slang.h>
#include HEADER_OPERAND

#define HEADER_WITH_SPACES <header-name/header with spaces.slang.h>
#include HEADER_WITH_SPACES

// Nested replacements use spacing at each invocation, including within a filename.
#define HEADER_NAME regular.slang.h
#define HEADER_ALIAS HEADER_NAME
#define NESTED_HEADER <header-name/HEADER_ALIAS>
#include NESTED_HEADER

#define HEADER_EXTENSION() slang
#define FUNCTION_HEADER <header-name/regular.HEADER_EXTENSION().h>
#include FUNCTION_HEADER

// Whitespace at the ends of an argument is discarded.
// We keep spacing written before a parameter in the replacement list.
#define HEADER_ID(x) x
#define ARGUMENT_HEADER <header-name/HEADER_ID( regular.slang.h )>
#include ARGUMENT_HEADER

#define SPACED_HEADER(x) <header-name/header x spaces.slang.h>
#include SPACED_HEADER(with)

// An empty expansion preserves the separator before its invocation.
#define EMPTY_HEADER_PART()
#define HEADER_AFTER_EMPTY <header-name/header EMPTY_HEADER_PART()with spaces.slang.h>
#include HEADER_AFTER_EMPTY

#define QUOTED_HEADER "header-name/regular.slang.h"
#include QUOTED_HEADER

// A macro argument can use `<` as an operator while another argument supplies the path.
#define SELECT_HEADER(condition, header) header
#include SELECT_HEADER(1 < 2, "header-name/regular.slang.h")

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
