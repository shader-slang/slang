// Here we are going to do a very simple calculation with double.
// For simplicity we are not going to use any intrinsics, and not use any resources that use double

//TEST:COMPARE_COMPUTE(targets=dx12+cuda+cpu+llvm):
//TEST:COMPARE_COMPUTE(targets=all-metal-wgpu):-render-feature double
//TEST:COMPARE_COMPUTE(targets=dx12):-profile sm_6_0
//TEST:COMPARE_COMPUTE(targets=vk):-render-feature double
// Not supported in WGSL: Double and other unsupported scalar types

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);

    double f = idx * (1.0l / (4.0l));

    f = 1.0l / f;

    {
        uint low, high;
        asuint(f * 2.0l, low, high);
        outputBuffer[idx * 2 + 0] = low;
        outputBuffer[idx * 2 + 1] = high;
    }
}
