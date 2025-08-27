//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv

RWStructuredBuffer<float> mutable_float_buffer;
RWStructuredBuffer<uint> mutable_uint_buffer;

StructuredBuffer<float> constant_float_buffer;
StructuredBuffer<uint> constant_uint_buffer;

// We do not allow taking a pointer from a StructuredBuffer/RWStructuredBuffer.
[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // CHECK: ([[# @LINE+1]]): error 31160
    float* mutablePtr = __getAddress(mutable_float_buffer[threadId.x]);

    InterlockedAdd(mutable_uint_buffer[threadId.x], 1);

    // CHECK: ([[# @LINE+1]]): error 31160
    float* ptr = __getAddress(constant_float_buffer[threadId.x]);


    InterlockedAdd(constant_uint_buffer[0], 1);
}
