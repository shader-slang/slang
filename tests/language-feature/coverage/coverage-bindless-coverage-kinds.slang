// The bindless form's two-level access runs for every counter update,
// whatever produced it. The other bindless tests only use line coverage, so
// this covers the combinations they leave out: branch counters, and the
// boolean recording mode that writes a plain store instead of an atomic.
//
// A lowering break specific to (bindless x branch) or (bindless x boolean)
// would otherwise ship silently, since the single-buffer form is tested
// across all of those kinds and the bindless form was not.

//TEST:SIMPLE(filecheck=BRANCH):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-branch-coverage -trace-coverage-binding 0 31 -trace-coverage-bindless-index 7
//TEST:SIMPLE(filecheck=BOOLEAN):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-coverage -trace-coverage-boolean -trace-coverage-binding 0 31 -trace-coverage-bindless-index 7

//TEST_INPUT: set outBuf = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        outBuf[0] = 1;
    }
    else
    {
        outBuf[0] = 2;
    }
}

// Branch counters reach the descriptor array the same way line counters do:
// select this shader's buffer out of the array, then index within it.
//BRANCH-DAG: OpCapability RuntimeDescriptorArray
//BRANCH-DAG: OpAccessChain {{.*}} %__slang_coverage %int_7
//BRANCH: OpAtomicIAdd

// Boolean mode writes a plain store rather than an atomic, and must still go
// through the array. If the two-level access were lost on this path the
// counter would land in whatever buffer sits at element 0.
//BOOLEAN-DAG: OpCapability RuntimeDescriptorArray
//BOOLEAN-DAG: OpAccessChain {{.*}} %__slang_coverage %int_7
//BOOLEAN-NOT: OpAtomicIAdd
