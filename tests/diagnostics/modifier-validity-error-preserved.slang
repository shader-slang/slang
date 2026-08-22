// shader-slang/slang#12558: `inline` is meaningful only on a callable, so applying it to a
// non-callable must be an error (E31201). This exercises both verdict paths: an aggregate type
// hits the authored `Error` rule (AggTypeDecl, InlineModifier), and a global variable matches no
// rule and is rejected by the legacy `checkModifierAllowedOnDecl` fallback.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

//CHECK: modifier 'inline' is not allowed here
inline struct NotACallable
{
    int x;
}

//CHECK: modifier 'inline' is not allowed here
inline int gNotACallable;

[numthreads(1, 1, 1)]
void computeMain() {}
