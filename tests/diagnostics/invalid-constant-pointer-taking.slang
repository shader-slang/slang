//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -stage compute -entry computeMain -target spirv

RWStructuredBuffer<float> mutable_float_buffer;
RWStructuredBuffer<uint> mutable_uint_buffer;

StructuredBuffer<float> constant_float_buffer;
StructuredBuffer<uint> constant_uint_buffer;

// Taking a pointer to a *mutable* structured-buffer element (via `&` or `__getAddress`)
// is allowed; taking one from a read-only `StructuredBuffer` is not (shader-slang/slang#12581).
[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    float* mutablePtr1 = &mutable_float_buffer[threadId.x];

    float* mutablePtr2 = __getAddress(mutable_float_buffer[threadId.x]);

    InterlockedAdd(mutable_uint_buffer[threadId.x], 1);

    // Slang does not support pointers to constant storage.
    float* ptr1 = &constant_float_buffer[threadId.x];
//diag:                                 ^ property or subscript does not provide an address
//diag:                                 ^ cannot take a direct reference to this subscript because it has no `ref` accessor

    float* ptr2 = __getAddress(constant_float_buffer[threadId.x]);
//diag:           ^^^^^^^^^^^^ invalid __getAddress usage
//diag:           ^^^^^^^^^^^^ '__getAddress' cannot take the address of a function-local variable on this target.
//diag:                                             ^ property or subscript does not provide an address
//diag:                                             ^ cannot take a direct reference to this subscript because it has no `ref` accessor

    InterlockedAdd(constant_uint_buffer[0], 1);
//diag:                                ^ property or subscript does not provide an address
//diag:                                ^ cannot take a direct reference to this subscript because it has no `ref` accessor
}
