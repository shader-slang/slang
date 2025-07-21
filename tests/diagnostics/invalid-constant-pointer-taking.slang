//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain -target spirv

RWStructuredBuffer<float> mutable_float_buffer;
RWStructuredBuffer<uint> mutable_uint_buffer;

StructuredBuffer<float> constant_float_buffer;
StructuredBuffer<uint> constant_uint_buffer;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    float* mutablePtr = &mutable_float_buffer[threadId.x];

    InterlockedAdd(mutable_uint_buffer[threadId.x], 1);

    // Constant pointers arent a thing in slang
    // CHECK: error 30079:
    float* ptr = &constant_float_buffer[threadId.x];


    InterlockedAdd(constant_uint_buffer[0], 1);
}