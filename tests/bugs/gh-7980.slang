//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl

// Arrays of resource types are illegal in WGSL because these buffer types map
// to runtime-sized arrays, and WGSL does not allow runtime-sized arrays as
// elements of fixed-size arrays.
// GitHub issue: https://github.com/shader-slang/slang/issues/7980

RWStructuredBuffer<float> result[2];
//CHECK:                  ^^^^^^ E56105
//CHECK:                  ^^^^^^ arrays of resource types
StructuredBuffer<float> input[2];
//CHECK:                ^^^^^ E56105
//CHECK:                ^^^^^ arrays of resource types
RWByteAddressBuffer raw[2];
//CHECK:            ^^^ E56105
//CHECK:            ^^^ arrays of resource types

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    uint index = threadId.x;
    result[0][index] = input[0][index];
    raw[0].Store(index * 4, index);
}
