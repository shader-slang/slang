// The `{CallableDecl, InlineModifier, Allowed}` rule takes precedence over the legacy check, so
// `inline` on a function must keep compiling with no diagnostic. Without this positive test a
// regression of that rule (wrong class or disposition) would silently start rejecting valid
// `inline` functions. Exhaustive mode (no `non-exhaustive`) fails on any spurious diagnostic on
// the function.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

inline int okInline(int a)
{
    return a + 1;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    int x = okInline(41);
}
