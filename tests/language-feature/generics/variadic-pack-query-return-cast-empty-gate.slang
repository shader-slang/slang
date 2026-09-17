//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// `__first`/`__last` are partial operations: applied to a value pack that is not known non-empty
// (no `where nonempty` constraint), they must be rejected with diagnostic code E30412. This asserts
// the code itself, not just the message text, so the empty-pack gate cannot silently regress.

int firstNoGate<each T>(expand each T args) where T == int
{
    return __first(args);
//CHECK:   ^^^^^^^ E30412
}

int lastNoGate<each T>(expand each T args) where T == int
{
    return __last(args);
//CHECK:   ^^^^^^ E30412
}
