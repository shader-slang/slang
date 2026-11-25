//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):

// A `?:` in a generic array size is folded once its operands are substituted. When the condition
// is constant, the size is the selected operand even if the other operand does not fold. For
// example, in `S<true, 32>` the size `B ? 1 : (1 << N)` is 1, although `1 << 32` is
// target-defined and stays unfolded. The array type must then be `float[1]`, so that it matches
// a `float[1]` declared directly.
//
// None of the cases may produce a diagnostic: the test runs in exhaustive mode with no
// annotations.

struct S<let B : bool, let N : int>
{
    float a[B ? 1 : (1 << N)];
}

void selectsTrueArm(S<true, 32> s)
{
    float x[1] = s.a;
}

void selectsFalseArm(S<false, 3> s)
{
    float x[8] = s.a;
}
