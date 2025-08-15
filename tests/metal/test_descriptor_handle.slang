//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target metal

Buffer<uint> inputBuffer;

RWStructuredBuffer<uint> outputBuffer;

struct MyStruct
{
    uint value;
}

// CHECK: texture_buffer<uint, access::read> inputBuffer_{{.*}}
ParameterBlock<DescriptorHandle<Buffer<uint>>> param;

[numthreads(4, 1, 1)]
void computeMain(uint3 dtid : SV_DispatchThreadID)
{
    uint idx = dtid.x;
    // Load values from the buffer to verify correct access

    outputBuffer[idx] = param[0];
}
