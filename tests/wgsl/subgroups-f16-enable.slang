//TEST:SIMPLE(filecheck=BALLOT): -target wgsl -entry ballotMain -stage compute
//TEST:SIMPLE(filecheck=SUM): -target wgsl -entry sumMain -stage compute
//TEST:SIMPLE(filecheck=NOEXT): -target wgsl -entry ballotMain -stage compute
//TEST:SIMPLE(filecheck=NOEXT): -target wgsl -entry sumMain -stage compute

// A shader that uses f16 values and subgroup operations needs only `enable f16;`
// and `enable subgroups;`. WGSL has no `subgroups_f16` extension (Dawn/Tint removed
// it), so it must not be emitted (#13636). ballotMain uses f16 only outside the
// subgroup operation; sumMain passes a `half` operand to the subgroup operation.
// NOEXT searches the whole output. (The split into these cases follows #13650.)

// NOEXT-NOT: subgroups_f16

StructuredBuffer<half> halves;
RWStructuredBuffer<float> outputBuffer;

// BALLOT: enable f16;
// BALLOT-NOT: subgroups_f16
// BALLOT: enable subgroups;
// BALLOT-NOT: subgroups_f16
// BALLOT: subgroupBallot
[numthreads(64, 1, 1)]
void ballotMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const uint4 ballot = WaveActiveBallot(dispatchThreadID.x < 10);
    outputBuffer[dispatchThreadID.x] = float(halves[dispatchThreadID.x]) + float(ballot.x);
}

// SUM: enable f16;
// SUM-NOT: subgroups_f16
// SUM: enable subgroups;
// SUM-NOT: subgroups_f16
// SUM: subgroupAdd
[numthreads(64, 1, 1)]
void sumMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = float(WaveActiveSum(halves[dispatchThreadID.x]));
}
