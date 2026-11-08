// Metal has no floating-point texture atomics. An exchange, min or max on a float texel is
// diagnosed exactly once, by the Metal checks in slang-ir-check-unsupported-inst.cpp.

//TEST:SIMPLE(filecheck=XCHG,exit-code=-1):-target metal -entry exchangeMain -stage compute
//TEST:SIMPLE(filecheck=MIN,exit-code=-1):-target metal -entry minMain -stage compute
//TEST:SIMPLE(filecheck=MAX,exit-code=-1):-target metal -entry maxMain -stage compute

[format("r32f")]
globallycoherent RWTexture2D<float> t;

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void exchangeMain(uint3 tid: SV_DispatchThreadID)
{
    float originalValue;
    InterlockedExchange(t[uint2(1, 0)], 1.0, originalValue);
    outputBuffer[0] = originalValue;
}

// XCHG: error[E55204]
// XCHG-NOT: E55204

[numthreads(1, 1, 1)]
void minMain(uint3 tid: SV_DispatchThreadID)
{
    InterlockedMin(t[uint2(1, 0)], 1.0);
}

// MIN: error[E55204]
// MIN-NOT: E55204

[numthreads(1, 1, 1)]
void maxMain(uint3 tid: SV_DispatchThreadID)
{
    InterlockedMax(t[uint2(1, 0)], 1.0);
}

// MAX: error[E55204]
// MAX-NOT: E55204
