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
    float* mutablePtr1 = &mutable_float_buffer[threadId.x];

    // CHECK: ([[# @LINE+1]]): error 31160
    float* mutablePtr2 = __getAddress(mutable_float_buffer[threadId.x]);

    InterlockedAdd(mutable_uint_buffer[threadId.x], 1);

    // Constant pointers arent a thing in slang
    // CHECK: ([[# @LINE+1]]): error 30079:
    float* ptr1 = &constant_float_buffer[threadId.x];

    // CHECK: ([[# @LINE+1]]): error 31160
    float* ptr2 = __getAddress(constant_float_buffer[threadId.x]);

    InterlockedAdd(constant_uint_buffer[0], 1);
}
