//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgpu -compute -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

// WaveActiveAllEqual on bool and bool vectors. WGSL's subgroupBroadcastFirst takes no bool, so
// the WGSL lowering compares the values as 0/1.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);

    int value = 0;

    value |= WaveActiveAllEqual(idx >= 0)                       ? 1 : 0;    // true
    value |= WaveActiveAllEqual((idx & 2) != 0)                 ? 2 : 0;    // false
    value |= WaveActiveAllEqual(bool2(true, idx < 4))           ? 4 : 0;    // true
    value |= WaveActiveAllEqual(bool2(true, (idx & 1) != 0))    ? 8 : 0;    // false
    value |= WaveActiveAllEqual(idx < 0)                        ? 16 : 0;   // true (all false)
    value |= WaveActiveAllEqual(bool2(false, idx >= 0))         ? 32 : 0;   // true (mixed, uniform)

    outputBuffer[idx] = value;
}

// 1 | 4 | 16 | 32 on every lane.
// CHECK: 53
// CHECK-NEXT: 53
// CHECK-NEXT: 53
// CHECK-NEXT: 53
