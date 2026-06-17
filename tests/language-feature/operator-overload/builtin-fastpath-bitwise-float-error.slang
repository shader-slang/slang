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

void test(float a, float b, double d0, double d1, int i, float3 v0, float3 v1)
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
    float3 r6 = v0 | v1; // component-wise float vector
    //CHECK: requires integer operands
}
