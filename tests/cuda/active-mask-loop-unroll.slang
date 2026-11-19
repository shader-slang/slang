// The CUDA active-mask pass passes the mask for a loop iteration to the loop
// header as a block argument, which rebuilds the `loop` instruction. The
// rebuilt instruction must keep the `[unroll]` attribute.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none -validate-ir

RWStructuredBuffer<uint> outputBuffer;

[numthreads(8, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint acc = 0;

    // CUDA: #pragma unroll
    // CUDA-NEXT: for(;;)
    [unroll]
    for (uint i = 0; i < 4; i++)
        acc += WaveActiveSum(i * outputBuffer[tid.x]);

    outputBuffer[tid.x] = acc;
}
