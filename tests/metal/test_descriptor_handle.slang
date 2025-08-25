//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target metal

struct MyStruct
{
    uint value;
}
// CHECK-NOT: DescriptorHandle
ParameterBlock<DescriptorHandle<Buffer<uint>>> param;
RWStructuredBuffer<uint> outputBuffer;
[numthreads(4, 1, 1)]
void computeMain(uint3 dtid : SV_DispatchThreadID)
{
    uint idx = dtid.x;
    // Load values from the buffer to verify correct access
    // CHECK: {{.*}}outputBuffer{{.*}}=
    outputBuffer[idx] = param[0];
}
