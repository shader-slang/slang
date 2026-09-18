//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute

// #13168: emittable snippet operands still lower to real SPIR-V. `const(uint, ...)` and
// `const(half, ...)` are emittable (the type-operand path already lowers `uint`/`half`), a
// `const(_p, ...)` on a float-returning intrinsic resolves to `float`, and a `_type(...)` naming an
// emittable type lowers on the type-operand path. emitSpvConstant / emitSpvSnippetASMTypeOperand
// emit an `OpConstant` / type for each, so the snippets below lower to `OpIAdd` / `OpFAdd` /
// `OpBitcast`. Two distinct `half` constants pin the cache-keying fix (see addQuarterHalf).

RWStructuredBuffer<float> outBuf;

__target_intrinsic(spirv, "OpIAdd resultType resultId _0 const(uint, 7)")
uint addSevenU(uint x);

__target_intrinsic(spirv, "OpFAdd resultType resultId _0 const(half, 0.5)")
half addHalfConst(half x);

// A second, distinct `half` constant: the constant cache keys on `floatValues`, so distinct half
// values must emit distinct `OpConstant`s rather than collapsing to a single shared one.
__target_intrinsic(spirv, "OpFAdd resultType resultId _0 const(half, 0.25)")
half addQuarterHalf(half x);

__target_intrinsic(spirv, "OpFAdd resultType resultId _0 const(_p, 0.5)")
float addFloatOrDoubleConst(float x);

// A `_type(...)` operand naming an emittable type lowers on the type-operand path.
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

// The disassembler prints half constants in hex (0.5 -> 0x1p-1, 0.25 -> 0x1p-2) but the float
// constant in decimal (0.5); the two half values are distinct, proving the cache does not collide.
//CHECK-DAG: OpConstant %uint 7
//CHECK-DAG: OpConstant %half 0x1p-1
//CHECK-DAG: OpConstant %half 0x1p-2
//CHECK-DAG: OpConstant %float 0.5
//CHECK-DAG: OpBitcast
//CHECK-DAG: OpIAdd
//CHECK-DAG: OpFAdd
