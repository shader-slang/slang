//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

extern static const bool kVariantMSSupported = false;

RWStructuredBuffer<uint> outputBuffer;
Conditional<RWTexture2DMSArray<uint>, kVariantMSSupported> headIndexImageMs;

// CHECK: OpName %[[OUTPUT:[A-Za-z_][A-Za-z_0-9]*]] "outputBuffer"
// CHECK: OpDecorate %[[OUTPUT]] Binding 0
// CHECK-NOT: headIndexImageMs

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = dispatchThreadID.x;
}
