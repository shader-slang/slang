//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
//
// Regression test for #11829: a quote-form `import` names a module/file path, not a string
// literal, so a backslash separator that begins what looks like an escape sequence (here
// `\u`) must be read raw -- with no escape decoding or validation -- the same way
// `#include` is read via getFileNameTokenValue. The module does not exist, so the only
// diagnostic is the path-not-found error carrying the raw path (module resolution appends
// the `.slang` extension); a re-introduced 10007/10008 escape error would surface as an
// unannotated diagnostic and fail this (exhaustive) test.

import "rtx\utility\common";
//CHECK: cannot open file 'rtx\utility\common.slang'

void main()
{
}
