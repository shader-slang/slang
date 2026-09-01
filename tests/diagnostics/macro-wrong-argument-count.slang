// macro-wrong-argument-count.slang
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Tests the wrong-argument-count diagnostic for function-like macros, for
// both the non-variadic and the variadic (too few required arguments)
// checks in _maybeBeginMacroInvocation. A mismatched invocation expands to
// zero tokens, which is why the surrounding statements stay parseable.
//
// These error paths also abandon the just-allocated MacroInvocation, so
// running this test under LSan pins the leak fix for both branches.

#define BAD_FIXED(X, Y) (X + Y)
#define BAD_VARIADIC(X, Y, ...) (X + Y)

int foo()
{
    return BAD_FIXED(1);
    //CHECK:        ^ error E15501
    //CHECK:        ^ wrong number of arguments to macro (expected 2, got 1)
}

int bar()
{
    return BAD_VARIADIC(1);
    //CHECK:           ^ error E15501
    //CHECK:           ^ wrong number of arguments to macro (expected 2, got 1)
}
