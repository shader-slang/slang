//TEST:SIMPLE(filecheck=SMALL):-target hlsl -entry computeMain -conformance A:IValue=0 -conformance B:IValue=1 -conformance C:IValue=2 -warnings-as-errors 55219
//DIAGNOSTIC_TEST:SIMPLE(diag=WARNING):-target hlsl -entry computeMain -conformance A:IValue=0 -conformance B:IValue=1 -conformance C:IValue=2 -DLARGE=1
//DIAGNOSTIC_TEST:SIMPLE(diag=ERROR):-target hlsl -entry computeMain -conformance A:IValue=0 -conformance B:IValue=1 -conformance C:IValue=2 -DLARGE=1 -warnings-as-errors 55219

interface IValue { uint get(); }
struct A : IValue { uint get() { return 2; } }
struct B : IValue { uint get() { return 5; } }
struct C : IValue { uint get() { return 7; } }

uint readMarker(inout uint state) { return state++; }

// Keep the calls in the helper body: the limit counts pre-inlining IR, not
// the work performed by callees. Three copies also exercise a non-divisor of 1024.
#define TEN_CALLS \
    state += readMarker(state); \
    state += readMarker(state); \
    state += readMarker(state); \
    state += readMarker(state); \
    state += readMarker(state); \
    state += readMarker(state); \
    state += readMarker(state); \
    state += readMarker(state); \
    state += readMarker(state); \
    state += readMarker(state);
#define HUNDRED_CALLS \
    TEN_CALLS TEN_CALLS TEN_CALLS TEN_CALLS TEN_CALLS \
    TEN_CALLS TEN_CALLS TEN_CALLS TEN_CALLS TEN_CALLS

[__specializePerConformance]
uint helper<T : IValue>(T value, inout uint state)
{
#if LARGE
    HUNDRED_CALLS HUNDRED_CALLS HUNDRED_CALLS
    HUNDRED_CALLS HUNDRED_CALLS HUNDRED_CALLS
#else
    TEN_CALLS
#endif
    return value.get() + state;
}

RWStructuredBuffer<uint> outputBuffer;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint state = 0;
    outputBuffer[tid.x] = helper(createDynamicObject<IValue>(tid.x, 0u), state);
}
// SMALL: numthreads
//WARNING: warning E55219
//WARNING: exceeds the current resource limit of 1024 duplicated IR instructions
//ERROR: error E55219
//ERROR: exceeds the current resource limit of 1024 duplicated IR instructions
