// Regression test for https://github.com/shader-slang/slang/issues/11648
//
// A bitwise/shift operator (`& | ^ << >>`) on a floating-point operand has no integer
// interpretation. The builtin-operator fast path now emits a dedicated error instead of letting it
// fall through to a confusing `ambiguous call`/`no overload` (#11648, regression of #11493).
// Front-end error, target-independent; `non-exhaustive` ignores the cascading errors.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void test(
    float a,
    float b,
    double d0,
    double d1,
    int i,
    float3 v0,
    float3 v1,
    float2x2 m0,
    float2x2 m1,
    half h0,
    half h1)
{
    float r0 = a | b;
    //CHECK: requires integer operands
    float r1 = a & b;
    //CHECK: requires integer operands
    float r2 = a ^ b;
    //CHECK: requires integer operands
    float r3 = a << b;
    //CHECK: requires integer operands
    float r4 = a | i; // mixed float/int promotes to float
    //CHECK: requires integer operands
    double r5 = d0 | d1;
    //CHECK: requires integer operands
    half r6 = h0 ^ h1; // 16-bit float element is still floating-point
    //CHECK: requires integer operands
    float3 r7 = v0 | v1; // component-wise float vector
    //CHECK: requires integer operands
    float2x2 r8 = m0 | m1; // component-wise float matrix
    //CHECK: requires integer operands
    // Mixed-type shifts skip common-type promotion but are still rejected (#11648).
    float r9 = a << i; // float << int
    //CHECK: requires integer operands
    int r10 = i << a; // int << float
    //CHECK: requires integer operands
    float r11 = ~a; // unary `~` rejected by the same diagnostic
    //CHECK: requires integer operands
    int r12 = a | b; // result/target type is irrelevant; the operand element type drives it
    //CHECK: requires integer operands

    // The original confusing symptoms must be gone.
    //CHECK-NOT: ambiguous
    //CHECK-NOT: no overload
}
