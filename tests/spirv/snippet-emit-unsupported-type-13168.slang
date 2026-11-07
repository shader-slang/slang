//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry computeMain -stage compute

// A `_type(...)` of an unemittable type, including `_p`, which has no result-type resolution, is
// diagnosed before emission.

RWStructuredBuffer<float> outBuf;

//CHECK: E29003
//CHECK: type operand of unsupported type 'double'
__target_intrinsic(spirv, "OpNop _type(double)")
float useDoubleType(float a);

//CHECK: type operand of unsupported type 'float-or-double (_p)'
__target_intrinsic(spirv, "OpNop _type(_p)")
float useFloatOrDoubleType(float a);

//CHECK: type operand of unsupported type 'unknown'
__target_intrinsic(spirv, "OpNop _type(notatype)")
float useUnknownType(float a);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = useDoubleType(1.0f) + useFloatOrDoubleType(2.0f) + useUnknownType(3.0f);
}
