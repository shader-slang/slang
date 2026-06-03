// Regression test for #11291 Bug 4: an unknown escape sequence such as
// `\c` was previously consumed silently — the post-backslash byte got
// emitted as plain text and the user saw no diagnostic. Both the string
// and char literal paths now surface a diagnostic.
//
// `#include`-style file paths (which use the lexer's string-literal token
// shape but go through `getFileNameTokenValue`) deliberately do not
// run the escape consumer, so Windows-style backslash separators in
// `#include "./a\b.h"` continue to work.

//DIAGNOSTIC_TEST:SIMPLE:

void foo()
{
    let s = "ab\cde";
    let c = '\c';
}
