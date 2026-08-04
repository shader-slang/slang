// Regression test for https://github.com/shader-slang/slang/issues/7857
//
// Negative companion to overload-argument-type-mismatch.slang. When a candidate
// is rejected only because of a qualifier/direction mismatch (the underlying
// types are equal), the per-candidate E40018 "argument N does not match:
// expected 'T', got 'T'" note must NOT be emitted -- that case has its own
// dedicated diagnostics. This pins the suppression in `recordArgMismatch`
// (`!paramType.type->equals(argType.type)`).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void f(out int x) { x = 0; }
void f(inout int x, int y) { x = y; }

void test()
{
    // `3` is not an l-value, so neither `out`/`inout` overload applies, but the
    // underlying parameter type (int) matches the argument type (int).
    f(3);
//CHECK: l-value
}

// The confusing "expected 'int', got 'int'" note must never appear.
//CHECK-NOT: E40018
//CHECK-NOT: does not match
