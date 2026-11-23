//TEST:SIMPLE(filecheck=CHECK): -target cubin-asm -entry computeMain -capability cuda_sm_8_0
//TEST:SIMPLE(filecheck=CHECK): -target cubin-asm -entry computeMain -capability cuda_sm_7_0 -Xnvrtc -arch=sm_80
//TEST:SIMPLE: -entry computeMain -capability cuda_sm_8_0 -o cubin-asm.slang.2.actual.cubin.asm
//TEST:SIMPLE: -entry computeMain -capability cuda_sm_8_0 -o cubin-asm.slang.3.actual.cubin-asm
//TEST:SIMPLE(filecheck=CHECK): -target cubin-asm -entry computeMain -capability cuda_sm_8_9 -Xnvrtc -arch=sm_80

// Check structural output rather than instructions, which vary with the toolkit optimizer.
// CHECK: .headerflags
// CHECK-SAME: EF_CUDA_SM80
// CHECK: computeMain

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = tid.x + 1;
}
