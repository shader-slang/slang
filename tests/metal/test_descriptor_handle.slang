//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target metal
//DISABLE_TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal -compute -entry computeMain -output-using-type

// FIXME: Compute test disabled due to:
// - No test input is provided for inputBuffer because TextureBuffer isn't
//   handled by the metal backend of the slang-test infrastructure.
// - Infinite loop in reflection

struct MyStruct
{
    uint value;
}
// CHECK-NOT: DescriptorHandle
ParameterBlock<DescriptorHandle<Buffer<uint>>> param;
//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;
[numthreads(4, 1, 1)]
void computeMain(uint3 dtid : SV_DispatchThreadID)
{
    uint idx = dtid.x;
    // Load values from the buffer to verify correct access
    // CHECK: {{.*}}outputBuffer{{.*}}=
    outputBuffer[idx] = param[0];
}
