//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute

// A shader that uses f16 values and subgroup operations needs only `enable f16;`
// and `enable subgroups;`. WGSL has no `subgroups_f16` extension, so it must not
// be emitted.

// CHECK-NOT: subgroups_f16
// CHECK-DAG: enable f16;
// CHECK-DAG: enable subgroups;
// CHECK-NOT: subgroups_f16
// CHECK: subgroupBallot
// CHECK: subgroupAdd

StructuredBuffer<half> halves;
RWStructuredBuffer<float> outputBuffer;

[numthreads(64, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const uint4 ballot = WaveActiveBallot(dispatchThreadID.x < 10);
    const half sum = WaveActiveSum(halves[dispatchThreadID.x]);
    outputBuffer[dispatchThreadID.x] = float(sum) + float(ballot.x);
}
