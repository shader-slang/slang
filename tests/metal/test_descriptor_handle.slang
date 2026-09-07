//TEST:SIMPLE(filecheck=METAL): -stage compute -entry computeMain -target metal

struct MyStruct
{
    uint value;
}
// The DescriptorHandle<Buffer<uint>> parameter must lower to a direct Metal resource binding: a
// texture_buffer field inside the ParameterBlock wrapper struct, accessed with no descriptor-handle
// indirection.
// METAL: texture_buffer<uint, access::read>
ParameterBlock<DescriptorHandle<Buffer<uint>>> param;
RWStructuredBuffer<uint> outputBuffer;
[numthreads(4, 1, 1)]
void computeMain(uint3 dtid : SV_DispatchThreadID)
{
    uint idx = dtid.x;
    // Load values from the buffer to verify correct access
    // METAL: {{.*}}outputBuffer{{.*}}=
    outputBuffer[idx] = param[0];
}
