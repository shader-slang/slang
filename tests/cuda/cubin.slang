//TEST:SIMPLE: -target cubin -entry computeMain -capability cuda_sm_8_0 -o cubin.slang.0.actual.cubin
//TEST:SIMPLE: -entry computeMain -capability cuda_sm_8_0 -o cubin.slang.1.actual.cubin
//TEST:SIMPLE: -target cubin -entry computeMain -Xnvrtc -arch=sm_80 -o cubin.slang.2.actual.cubin
//TEST:SIMPLE: -target cubin -entry computeMain -Xnvrtc --gpu-architecture=sm_80 -o cubin.slang.3.actual.cubin
//TEST:SIMPLE: -target cubin -entry computeMain -Xnvrtc -arch -Xnvrtc sm_80 -o cubin.slang.4.actual.cubin
//TEST:SIMPLE: -target cubin -entry computeMain -Xnvrtc --gpu-architecture -Xnvrtc sm_80 -o cubin.slang.5.actual.cubin

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = tid.x + 1;
}
