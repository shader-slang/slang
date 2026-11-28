// SPIR-V and GLSL have integer atomic compare-exchange only. A floating-point one is done on an
// integer view of the same memory (see atomic-float-compare-exchange.slang), which needs the
// destination to be in a `RWStructuredBuffer` (or, on SPIR-V, a physical storage buffer). Other
// destinations, and 16-bit values, report E55204 instead of producing invalid code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -target glsl -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;
RWStructuredBuffer<Atomic<half>> halfBuffer;
RWTexture2D<float> floatTexture;
groupshared Atomic<float> sharedAtomic;
groupshared float sharedFloat;

[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    float result = 0.0f;
    float original;

    result += sharedAtomic.compareExchange(0.0f, 2.0f);
//CHECK:                                  ^ floating point atomic compare-exchange outside a RWStructuredBuffer
    InterlockedCompareExchange(sharedFloat, 0.0f, 4.0f, original);
//CHECK:                      ^ floating point atomic compare-exchange outside a RWStructuredBuffer
    result += original;
    InterlockedCompareExchange(floatTexture[uint2(0, 0)], 0.0f, 5.0f, original);
//CHECK:                      ^ floating point atomic compare-exchange outside a RWStructuredBuffer
    result += original;
    result += float(halfBuffer[0].compareExchange(half(0.0), half(1.0)));
//CHECK:                                         ^ 16-bit floating point atomic compare-exchange

    outputBuffer[tid.x] = result;
}
