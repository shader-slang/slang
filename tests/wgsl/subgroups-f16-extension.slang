// Regression test for shader-slang/slang#13636. Dawn/Tint removed the `subgroups_f16` enable-extension,
// so f16 subgroup code enables only `f16` and `subgroups`. ballotMain uses f16 only outside the subgroup
// op (through a global buffer); sumMain passes a `half` operand to the subgroup op.

//TEST:SIMPLE(filecheck=BALLOT):-target wgsl -entry ballotMain -stage compute
//TEST:SIMPLE(filecheck=SUM):-target wgsl -entry sumMain -stage compute
//TEST:SIMPLE(filecheck=NOEXT):-target wgsl -entry ballotMain -stage compute
//TEST:SIMPLE(filecheck=NOEXT):-target wgsl -entry sumMain -stage compute

// NOEXT-NOT: subgroups_f16

StructuredBuffer<half> halves;
RWStructuredBuffer<float> outBuf;

// BALLOT: enable f16;
// BALLOT-NOT: subgroups_f16
// BALLOT: enable subgroups;
// BALLOT-NOT: subgroups_f16
// BALLOT: subgroupBallot
[numthreads(64, 1, 1)]
void ballotMain(uint3 tid : SV_DispatchThreadID)
{
    const uint4 b = WaveActiveBallot(tid.x < 10);
    outBuf[tid.x] = float(halves[tid.x]) + float(b.x);
}

// SUM: enable f16;
// SUM-NOT: subgroups_f16
// SUM: enable subgroups;
// SUM-NOT: subgroups_f16
// SUM: subgroupAdd
[numthreads(64, 1, 1)]
void sumMain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float(WaveActiveSum(halves[tid.x]));
}
