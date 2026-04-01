//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -specialize int -specialize float -specialize bool -stage compute

// Providing more specialization arguments than a generic entry point requires
// should produce error 38025.

RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain<T, U>(uint3 id : SV_DispatchThreadID)
{
    T x;
    U y;
}
//CHECK: wrong number of specialization arguments
//CHECK: expected 2 specialization arguments (3 provided)
