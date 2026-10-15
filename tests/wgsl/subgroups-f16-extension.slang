// Regression test for shader-slang/slang#13636. WGSL has no `subgroups_f16` extension: a module that
// uses f16 and subgroup operations enables only `f16` and `subgroups`.

//TEST:SIMPLE(filecheck=BALLOT):-target wgsl -entry ballotMain -stage compute
//TEST:SIMPLE(filecheck=SUM):-target wgsl -entry sumMain -stage compute

StructuredBuffer<half> halves;
RWStructuredBuffer<float> outBuf;

// BALLOT-NOT: subgroups_f16
// BALLOT: enable f16;
// BALLOT: enable subgroups;
// BALLOT-NOT: subgroups_f16
// BALLOT: subgroupBallot
[numthreads(64, 1, 1)]
void ballotMain(uint3 tid : SV_DispatchThreadID)
{
    const uint4 b = WaveActiveBallot(tid.x < 10);
    outBuf[tid.x] = float(halves[tid.x]) + float(b.x);
}

// SUM-NOT: subgroups_f16
// SUM: enable f16;
// SUM: enable subgroups;
// SUM-NOT: subgroups_f16
// SUM: subgroupAdd
[numthreads(64, 1, 1)]
void sumMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float(WaveActiveSum(halves[tid.x]));
}
