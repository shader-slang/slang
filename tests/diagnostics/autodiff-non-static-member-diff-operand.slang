//DISABLE_TEST:SIMPLE(diag=CHECK):

// Test that error 30098 should be emitted when a non-static member function
// is used as an operand to fwd_diff.
//
// BUG: Diagnostic 30098 is defined in slang-diagnostics.lua but never
// emitted. Instead, __fwd_diff on a non-static member function triggers
// a crash (segfault). This test is disabled until the diagnostic is
// properly wired up.

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
