//TEST:SIMPLE(filecheck=EIGHT):-target hlsl -entry computeMain -conformance N0:ICode=0 -conformance N1:ICode=1 -conformance N2:ICode=2 -conformance N3:ICode=3 -conformance N4:ICode=4 -conformance N5:ICode=5 -conformance N6:ICode=6 -conformance N7:ICode=7 -warnings-as-errors 55219
//DIAGNOSTIC_TEST:SIMPLE(diag=NINE,non-exhaustive):-target hlsl -entry computeMain -conformance N0:ICode=0 -conformance N1:ICode=1 -conformance N2:ICode=2 -conformance N3:ICode=3 -conformance N4:ICode=4 -conformance N5:ICode=5 -conformance N6:ICode=6 -conformance N7:ICode=7 -conformance N8:ICode=8
//DIAGNOSTIC_TEST:SIMPLE(diag=NINE,non-exhaustive):-target hlsl -entry computeMain -conformance N0:ICode=0 -conformance N1:ICode=1 -conformance N2:ICode=2 -conformance N3:ICode=3 -conformance N4:ICode=4 -conformance N5:ICode=5 -conformance N6:ICode=6 -conformance N7:ICode=7 -conformance N8:ICode=8 -warnings-as-errors 55219
// Diagnostic coverage below checks the same resource limit separately.


interface ICode
{
    static uint code();
}

struct N0 : ICode { static uint code() { return 0; } }
struct N1 : ICode { static uint code() { return 1; } }
struct N2 : ICode { static uint code() { return 2; } }
struct N3 : ICode { static uint code() { return 3; } }
struct N4 : ICode { static uint code() { return 4; } }
struct N5 : ICode { static uint code() { return 5; } }
struct N6 : ICode { static uint code() { return 6; } }
struct N7 : ICode { static uint code() { return 7; } }
struct N8 : ICode { static uint code() { return 8; } }

uint readMarker(inout uint state)
{
    return state++;
}

// Nine conformers exceed the current duplication budget. The hint must leave a
// single shared helper and preserve its behavior rather than rejecting the program.
[__specializePerConformance]
void getCode<T : ICode>(T value, inout uint state)
{
    uint context = readMarker(state);
    state = T.code() + context;
}

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(9, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint state = 0;
    getCode(createDynamicObject<ICode>(tid.x, 0u), state);
    outputBuffer[tid.x] = state;
}


// EIGHT-COUNT-8: = readMarker
// NINE: exceeds the current resource limit of 8 concrete copies
