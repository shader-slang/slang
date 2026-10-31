// Atomic<float> add/min/max/exchange on WebGPU (compare-exchange loops on the bits) give the exact
// results: 64 threads add 0, 0.5, ..., 31.5.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -wgpu -shaderobj -output-using-type

//TEST_INPUT: set sums = ubuffer(data=[0.0 100.0 -100.0 0.0], stride=4)
RWStructuredBuffer<Atomic<float>> sums;
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;
groupshared Atomic<float> groupSum;

[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID, uint gi: SV_GroupIndex)
{
    if (gi == 0)
        groupSum.store(0.0);
    GroupMemoryBarrierWithGroupSync();

    float v = float(tid.x) * 0.5; // 0, 0.5, ..., 31.5: exact sums
    sums[0].add(v);
    sums[1].min(v);
    sums[2].max(v);
    groupSum.add(1.0);
    GroupMemoryBarrierWithGroupSync();

    if (tid.x == 0)
    {
        float before = sums[3].exchange(7.0);
        // CHECK: {{^}}1008.0
        // CHECK-NEXT: {{^}}0.0
        // CHECK-NEXT: {{^}}31.5
        // CHECK-NEXT: {{^}}64.0
        // CHECK-NEXT: {{^}}0.0
        outputBuffer[0] = sums[0].load();
        outputBuffer[1] = sums[1].load();
        outputBuffer[2] = sums[2].load();
        outputBuffer[3] = groupSum.load();
        outputBuffer[4] = before;
    }
}
