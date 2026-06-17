// Regression test for https://github.com/shader-slang/slang/issues/11648
//
// A bitwise/shift operator (`& | ^ << >>`) applied to a floating-point operand has no valid
// integer interpretation. Before the fix, the builtin-operator fast path
// (`convertToBuiltinArithmeticOp`) declined these operands *silently* (`return nullptr`), so
// they fell through to normal overload resolution where `float` converted to every integer
// `operator|` overload at equal conversion cost, producing a confusing
// `error[E39999]: ambiguous call to '|' ...` (#11648, regression of #11493). The fast path now
// emits a dedicated, actionable error for genuine builtin float/double operands instead.
//
// Front-end (semantic) error, target-independent; `non-exhaustive` so cascading errors from the
// error-typed results are ignored.
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
    // Mixed-type shifts skip common-type promotion, but a float operand on either side is still
    // rejected here rather than falling through to a confusing "no overload" error (#11648).
    float r9 = a << i; // float << int
    //CHECK: requires integer operands
    int r10 = i << a; // int << float
    //CHECK: requires integer operands
    // Unary `~` on a float operand is rejected by the same diagnostic, instead of a confusing
    // "no overload for 'operator~'".
    float r11 = ~a;
    //CHECK: requires integer operands
    // The result/assignment-target type is irrelevant: the diagnostic keys off the *operand*
    // element type, so a float bitwise op fails even when assigned to an `int`.
    int r12 = a | b;
    //CHECK: requires integer operands

    // The original confusing symptoms must be gone: no ambiguous-overload error for `|`/`&`/`^`
    // and no "no overload" error for the mixed-type float shifts.
    //CHECK-NOT: ambiguous
    //CHECK-NOT: no overload
}
