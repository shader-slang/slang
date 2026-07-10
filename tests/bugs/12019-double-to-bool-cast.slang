// 12019-double-to-bool-cast.slang
// Test that casting a floating-point value to `bool` produces valid SPIR-V
// when emitting directly (`-emit-spirv-directly`).
// https://github.com/shader-slang/slang/issues/12019
//
// A float-to-bool cast lowers to `x != 0`, i.e. an `OpFUnordNotEqual`
// against a synthesized floating-point zero constant. The zero constant
// must be a proper float literal of the source type. For `double`, a
// well-formed `OpConstant` requires TWO 32-bit literal words; a malformed
// one-word constant makes the module structurally invalid and is rejected
// by spirv-val. These tests compile with SPIR-V validation enabled in CI
// (SLANG_RUN_SPIRV_VALIDATION=1), so an invalid module makes the compile
// fail with a non-zero result code. The `result code = 0` check confirms
// the module is valid, and `OpFUnordNotEqual` confirms the cast actually
// lowered (rather than being folded away). We check the `result code`
// literally rather than a `-NOT: error` guard, because the harness always
// prints the `standard error = {` label whether or not there was an error.
//
// Inputs are read from buffers (not literals) so the cast cannot be
// constant-folded away, guaranteeing the comparison against zero survives.

//TEST:SIMPLE(filecheck=DOUBLE): -target spirv-asm -stage compute -entry computeMainDouble -emit-spirv-directly
//TEST:SIMPLE(filecheck=FLOAT): -target spirv-asm -stage compute -entry computeMainFloat -emit-spirv-directly
//TEST:SIMPLE(filecheck=HALF): -target spirv-asm -stage compute -entry computeMainHalf -emit-spirv-directly

RWStructuredBuffer<uint> outputBuffer;
StructuredBuffer<double> inputDouble;
StructuredBuffer<float> inputFloat;
StructuredBuffer<half> inputHalf;

// DOUBLE: result code = 0
// DOUBLE: OpFUnordNotEqual
[numthreads(1, 1, 1)]
void computeMainDouble()
{
    outputBuffer[0] = (uint)(bool)inputDouble[0];
}

// FLOAT: result code = 0
// FLOAT: OpFUnordNotEqual
[numthreads(1, 1, 1)]
void computeMainFloat()
{
    outputBuffer[0] = (uint)(bool)inputFloat[0];
}

// HALF: result code = 0
// HALF: OpFUnordNotEqual
[numthreads(1, 1, 1)]
void computeMainHalf()
{
    outputBuffer[0] = (uint)(bool)inputHalf[0];
}
