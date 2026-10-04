// `CompilerOptionSet::writeCommandLineArgs` reconstructs the compile options
// into a command-line string, which SPIR-V debug info embeds so a tool can
// recover how a shader was built. The bindless index has to survive that
// round-trip: a shader's counter buffer is meaningless without knowing which
// array element it was compiled for.
//
// Nothing exercised that reconstruction for any of the coverage placement
// options -- the `-trace-coverage-reserved-space` case sitting beside it in
// the same switch is likewise unreachable from any test -- so a case that
// dropped or mangled the value would be silent.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -g2 -trace-coverage -trace-coverage-bindless-index 7

//TEST_INPUT: set outBuf = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = (float)tid.x;
}

// The reconstructed command line carries the option and its value together.
// A case emitting the flag without its operand, or the wrong operand field,
// would fail here.
//CHECK: -trace-coverage-bindless-index 7
