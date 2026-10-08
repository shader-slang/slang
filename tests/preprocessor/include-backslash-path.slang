// Regression test for https://github.com/shader-slang/slang/issues/11829.
//
// A quote-form `#include` whose path uses Windows-style backslash separators
// must not be rejected as an invalid string escape. Include paths are consumed
// raw -- getFileNameTokenValue deliberately does not decode escapes, because
// `\` is a path separator on Windows -- so the lexer must not validate escape
// sequences inside them. Before the fix, a `\u`/`\U`/`\x` sequence in the path
// (e.g. `\utility`, `\Users`, `\xtra`) triggered error 10007/10008 at lex time.
//
// This runs in exhaustive diagnostic mode: every emitted diagnostic must be
// annotated below, so a re-introduced escape error (10007/10008) would appear
// as an unannotated diagnostic and fail the test. The only diagnostics expected
// are the ordinary "file not found" ones, once each path is taken literally.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// `\u` is the dominant real-world trigger (e.g. "\utility", "\users").
#include "rtx\utility\common.slangh"
//CHECK: include file not found
//CHECK: failed to find include file 'rtx\utility\common.slangh'

// Also exercise `\U` and `\x` path components.
#include "dir\Users\f.slangh"
//CHECK: include file not found
//CHECK: failed to find include file 'dir\Users\f.slangh'

#include "a\xtra\b.slangh"
//CHECK: include file not found
//CHECK: failed to find include file 'a\xtra\b.slangh'
