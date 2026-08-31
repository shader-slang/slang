// Regression test for shader-slang/slang#12847.
//
// Verifies the SPIR-V codegen idiom for WavePrefixCountBits and
// WaveActiveCountBits: each builds a subgroup ballot feeding an
// OpGroupNonUniformBallotBitCount — ExclusiveScan for the prefix count, Reduce
// for the active count — and no private "__mask" register name leaks into OpName.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly

// CHECK-NOT: __mask
// CHECK-DAG: OpGroupNonUniformBallot %v4uint
// CHECK-DAG: OpGroupNonUniformBallotBitCount {{.*}} ExclusiveScan
// CHECK-DAG: OpGroupNonUniformBallotBitCount {{.*}} Reduce

RWStructuredBuffer<uint> outputBuffer;

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    bool p = (tid.x & 5) != 0;
    uint prefix = WavePrefixCountBits(p);
    uint active = WaveActiveCountBits(p);
    outputBuffer[tid.x] = prefix + active;
}
