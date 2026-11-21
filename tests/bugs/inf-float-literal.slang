// WGSL and Metal have no double, so the double case runs only where SUPPORT_DOUBLE is defined.
//TEST:COMPARE_COMPUTE(targets=all-wgpu-metal):-xslang -DSUPPORT_DOUBLE
//TEST:COMPARE_COMPUTE(targets=wgpu+metal):


//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int idx = int(dispatchThreadID.x);

    float a;

    switch (idx)
    {
        default:
        case 0:     a = 1.#INF; break;
        case 1:     a = 2.4#INFf; break;
#ifdef SUPPORT_DOUBLE
        case 2:     a = float(234.5#INFl); break;
#endif
        case 3:     a = -1.#INF; break;
    }

    outputBuffer[idx] = a;
}
