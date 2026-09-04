//DISABLE_DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test that error 30098 should be emitted when a non-static member function
// on a *concrete* receiver (e.g. `foo.compute` for `foo : Foo`) is used as
// an operand to fwd_diff.
//
// BUG: Diagnostic 30098 is defined in slang-diagnostics.lua but never
// emitted. Instead, __fwd_diff on a non-static member function on a
// concrete receiver still triggers a compiler assertion. Test remains
// disabled until the diagnostic is properly wired up.
//
// Note: shader-slang/slang#11004 fixed the generic-constrained variant of
// this lowering (e.g. `__fwd_diff(curve.eval)` for `curve : C, C : ICurve`);
// see tests/autodiff/fwd-diff-generic-constrained-member.slang. The
// concrete-receiver shape below reaches a different lowering path and is
// tracked separately.

struct Foo
{
    float x;

    [Differentiable]
    float compute(float t)
    {
        return x * t;
    }
}

void test()
{
    Foo foo;
    foo.x = 1.0;
    __fwd_diff(foo.compute)(DifferentialPair<float>(1.0, 1.0));
//CHECK:       ^^^^^^^^^^^ non-static function reference not allowed
//CHECK:       ^^^^^^^^^^^ non-static function reference 'compute' is not allowed here.
}
