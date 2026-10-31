//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry computeMain -stage compute

// A `const(...)` of an unemittable type, including a `_p` that resolves to `double` or stays
// unresolved, is diagnosed before emission.

RWStructuredBuffer<float> outBuf;

//CHECK: E29003
//CHECK: constant operand of unsupported type 'double'
__target_intrinsic(spirv, "OpFAdd resultType resultId _0 const(double, 0.5)")
double doubleConst(double a);

//CHECK: constant operand of unsupported type 'double'
__target_intrinsic(spirv, "OpFAdd resultType resultId _0 const(_p, 0.5)")
double pConstOnDouble(double a);

// On an `int` result, `_p` stays unresolved.
//CHECK: constant operand of unsupported type 'float-or-double (_p)'
__target_intrinsic(spirv, "OpNop const(_p, 1)")
int pConstOnInt(int a);

//CHECK: constant operand of unsupported type 'unknown'
__target_intrinsic(spirv, "OpNop const(notatype, 1)")
float unknownConst(float a);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = float(doubleConst(1.0)) + float(pConstOnDouble(2.0)) +
                float(pConstOnInt(4)) + unknownConst(3.0f);
}
