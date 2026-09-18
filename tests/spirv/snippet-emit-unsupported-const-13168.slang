//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry computeMain -stage compute

// #13168: a `const(...)` operand whose type the SPIR-V emitter cannot lower is diagnosed (E29001)
// during legalization instead of yielding a null operand at emit. A `const(_p, ...)` is first
// resolved from the intrinsic's result type: on a double-returning intrinsic it resolves to
// `double` (no constant lowering, rejected), and on a result that is neither float nor double it
// passes through unresolved as `float-or-double (_p)` (also rejected). Both are shown here, along
// with a directly-named `const(double, ...)` and an unrecognized constant type token (the `None`
// sentinel). (A float-resolving `_p` and `const(uint/half, ...)` are emitted; see
// snippet-const-emit-13168.slang.)

RWStructuredBuffer<float> outBuf;

//CHECK: E29001
//CHECK: constant operand of type 'double'
__target_intrinsic(spirv, "OpFAdd resultType resultId _0 const(double, 0.5)")
double doubleConst(double a);

//CHECK: constant operand of type 'double'
__target_intrinsic(spirv, "OpFAdd resultType resultId _0 const(_p, 0.5)")
double pConstOnDouble(double a);

// `_p` on an int-returning intrinsic resolves to neither float nor double, so it passes through as
// the unresolved `FloatOrDouble` placeholder and is rejected (guards resolveSnippetConstantType's
// pass-through branch).
//CHECK: constant operand of type 'float-or-double (_p)'
__target_intrinsic(spirv, "OpNop const(_p, 1)")
int pConstOnInt(int a);

//CHECK: constant operand of type 'unknown'
__target_intrinsic(spirv, "OpNop const(notatype, 1)")
float unknownConst(float a);

//CHECK-NOT: E99997
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = float(doubleConst(1.0)) + float(pConstOnDouble(2.0)) +
                float(pConstOnInt(4)) + unknownConst(3.0f);
}
