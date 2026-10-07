//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute

// Emittable operands (`const(uint|half)`, a float-resolving `const(_p)`, `_type(uint)`) lower to
// SPIR-V, and distinct `half` constants are not merged by the constant cache.

RWStructuredBuffer<float> outBuf;

__target_intrinsic(spirv, "OpIAdd resultType resultId _0 const(uint, 7)")
uint addSevenU(uint x);

__target_intrinsic(spirv, "OpFAdd resultType resultId _0 const(half, 0.5)")
half addHalfConst(half x);

__target_intrinsic(spirv, "OpFAdd resultType resultId _0 const(half, 0.25)")
half addQuarterHalf(half x);

__target_intrinsic(spirv, "OpFAdd resultType resultId _0 const(_p, 0.5)")
float addFloatOrDoubleConst(float x);

__target_intrinsic(spirv, "OpBitcast _type(uint) resultId _0")
uint reinterpretAsUInt(float x);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = float(addSevenU(uint(outBuf[0])));
    outBuf[1] = float(addHalfConst(half(outBuf[1])));
    outBuf[2] = float(addQuarterHalf(half(outBuf[2])));
    outBuf[3] = addFloatOrDoubleConst(outBuf[3]);
    outBuf[4] = float(reinterpretAsUInt(outBuf[4]));
}

// Half constants are printed in hex: 0x1p-1 is 0.5 and 0x1p-2 is 0.25.
//CHECK-DAG: OpConstant %uint 7
//CHECK-DAG: OpConstant %half 0x1p-1
//CHECK-DAG: OpConstant %half 0x1p-2
//CHECK-DAG: OpConstant %float 0.5
//CHECK-DAG: OpBitcast
//CHECK-DAG: OpIAdd
//CHECK-DAG: OpFAdd
