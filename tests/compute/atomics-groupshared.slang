// atomics-groupshared.slang

//TEST(compute):COMPARE_COMPUTE(targets=all-cpu-wgpu-llvm):-shaderobj
//TEST:COMPARE_COMPUTE(targets=metal):
// Not supported in WGSL: Use of traditional atomics intrinsics (InterlockedXXX functions)
// Not supported in LLVM: atomics are not currently available

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer

RWStructuredBuffer<uint> outputBuffer;

groupshared uint shared[4];

uint test(uint val)
{
    uint originalValue;

    shared[val] = 0;

    GroupMemoryBarrierWithGroupSync();

    uint originalSum = 0;

    InterlockedAdd(shared[val],         val,         originalValue);
    originalSum += originalValue;

    InterlockedAdd(shared[val ^ 1],     val*16,     originalValue);
    originalSum += originalValue;

    InterlockedAdd(shared[val ^ 2],     val*16*16,     originalValue);
    originalSum += originalValue;

    GroupMemoryBarrierWithGroupSync();

    return shared[val] ^ originalSum;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    uint val = test(tid);
    outputBuffer[tid] = val;
}
