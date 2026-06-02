// TEST:SIMPLE(filecheck=METAL4): -target metal -capability metallib_4_0
// TEST:SIMPLE(filecheck=METAL3): -target metal -capability metallib_3_1

RWStructuredBuffer<int> outputBuffer;

// Ensure that metal4 compilation includes the required_threads_per_threadgroup annotation.
// METAL3-NOT: {{\[\[}}required_threads_per_threadgroup(32, 16, 1){{\]\]}}
// METAL4: {{\[\[}}required_threads_per_threadgroup(32, 16, 1){{\]\]}}

[numthreads(32, 16, 1)]
void computeMain(uint3 dtid: SV_DispatchThreadID)
{
    uint idx = dtid.y;
    if (dtid.x == idx)
    {
        outputBuffer[idx] = (int)idx;
    }
}
