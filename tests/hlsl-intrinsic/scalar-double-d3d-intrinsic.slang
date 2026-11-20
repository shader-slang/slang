//TEST:COMPARE_COMPUTE(targets=cuda+cpu+llvm):-output-using-type
// On dx11 this causes a crash on CI (not currently not repeatable on test systems)
// This is disabled because Dx12/DXBC doing double writes to a structured buffer can fail
//TEST:COMPARE_COMPUTE(targets=dx12):-profile sm_6_0 -output-using-type -render-feature hardware-device
//TEST:COMPARE_COMPUTE(targets=vk):-output-using-type -render-feature double

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<double> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);

    double f = idx * (1.0f / (4.0f));

    int it = 0;
    double ft = 0.0l;

    // We known fmod is NOT supported on double on DX - only float is supported
    // ft += int(((f % 0.11l) * 100) + 0.5l);

    ft += rcp(1.0l + f);
    ft += sign(f - 0.5l);

    ft += saturate(f * 4 - 2.0l);

    ft += abs(f * 4 - 2.0l);

    ft += min(0.5l, f);
    ft += max(f, 0.75l);

    ft += clamp(f, 0.1l, 0.3l);

    {
        uint low, high;
        asuint(f * 2.0l, low, high);
        ft += asdouble(low, high);
    }

    outputBuffer[idx] = ft;
}
