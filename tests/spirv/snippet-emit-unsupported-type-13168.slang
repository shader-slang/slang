//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry computeMain -stage compute

// #13168: the SPIR-V emitter lowers only a fixed set of snippet type operands (isEmittableASMType).
// A `_type(...)` naming a type outside that set -- `double`, the `_p` float-or-double placeholder
// (a type operand has no result-type resolution), or an unrecognized token (mapped to the `None`
// sentinel by parseASMType) -- is diagnosed (E29001) during SPIR-V legalization, before it reaches
// the emitter. `__target_intrinsic` is in baseLanguageScope, so such a snippet is reachable from
// ordinary user code.

RWStructuredBuffer<float> outBuf;

//CHECK: E29001
//CHECK: type operand of type 'double'
__target_intrinsic(spirv, "OpNop _type(double)")
float useDoubleType(float a);

//CHECK: type operand of type 'float-or-double (_p)'
__target_intrinsic(spirv, "OpNop _type(_p)")
float useFloatOrDoubleType(float a);

//CHECK: type operand of type 'unknown'
__target_intrinsic(spirv, "OpNop _type(notatype)")
float useUnknownType(float a);

//CHECK-NOT: E99997
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = useDoubleType(1.0f) + useFloatOrDoubleType(2.0f) + useUnknownType(3.0f);
}
