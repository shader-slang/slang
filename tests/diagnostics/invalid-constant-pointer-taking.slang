//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -stage compute -entry computeMain -target spirv

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

    float* mutablePtr2 = __getAddress(mutable_float_buffer[threadId.x]);
//diag:                  ^^^^^^^^^^^^ invalid __getAddress usage
//diag:                  ^^^^^^^^^^^^ '__getAddress' only supports groupshared variables and members of groupshared/device memory.

    InterlockedAdd(mutable_uint_buffer[threadId.x], 1);

    // Constant pointers arent a thing in slang
    float* ptr1 = &constant_float_buffer[threadId.x];
//diag:                                 ^ cannot take address of immutable object
//diag:                                 ^ Not allowed to take the address of an immutable object
//diag:                                 ^ argument must be l-value
//diag:                                 ^ argument passed to parameter '0' must be l-value.

    float* ptr2 = __getAddress(constant_float_buffer[threadId.x]);
//diag:           ^^^^^^^^^^^^ invalid __getAddress usage
//diag:           ^^^^^^^^^^^^ '__getAddress' only supports groupshared variables and members of groupshared/device memory.

    InterlockedAdd(constant_uint_buffer[0], 1);
//diag:                                ^ argument must be l-value
//diag:                                ^ argument passed to parameter '0' must be l-value.
}
