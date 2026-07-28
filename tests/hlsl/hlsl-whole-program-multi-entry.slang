// A whole-program HLSL compile puts every entry point into one translation unit, so the
// `[shader("...")]` attribute is the only thing left that tells DXC which function is an
// entry point and what stage it has. Check that the attributes survive, that declarations
// shared between entry points are emitted once rather than once per entry, and that a
// single entry point named on the command line still leaves the attribute off.

//TEST:SIMPLE(filecheck=WHOLE): -target hlsl -whole-program
//TEST:SIMPLE(filecheck=WHOLE): -target hlsl -o -
//TEST:SIMPLE(filecheck=WHOLE): -target hlsl -whole-program -profile sm_6_5
//TEST:SIMPLE(filecheck=HELPER): -target hlsl -whole-program
//TEST:SIMPLE(filecheck=PAYLOAD): -target hlsl -whole-program
//TEST:SIMPLE(filecheck=ENTRIES): -target hlsl -entry MainCH -entry MainAH -o -
//TEST:SIMPLE(filecheck=SM51): -target hlsl -entry MainCS -profile cs_5_1
//TEST:SIMPLE(filecheck=WHOLESM51): -target hlsl -whole-program -profile sm_5_1

// Every entry point reaches the output carrying its stage. The `-o -` run covers the
// implicit whole-program path that `-o` now selects for HLSL, and the `sm_6_5` run covers
// an explicit shader model that can accept the attribute on its own.
// WHOLE: [shader("compute")]
// WHOLE: MainCS
// WHOLE: [shader("closesthit")]
// WHOLE: MainCH
// WHOLE: [shader("anyhit")]
// WHOLE: MainAH

// Naming the entry points explicitly instead of letting `[shader(...)]` find them selects
// the same whole-program output, so only the two named stages appear.
// ENTRIES: [shader("closesthit")]
// ENTRIES: MainCH
// ENTRIES: [shader("anyhit")]
// ENTRIES: MainAH

// Declarations shared between entry points are emitted once, not once per entry. Each
// pattern needs its own run so the -NOT reaches the end of the output.
// HELPER: uint sharedHelper{{[_0-9]*}}(uint
// HELPER-NOT: uint sharedHelper{{[_0-9]*}}(uint

// PAYLOAD: struct Payload
// PAYLOAD-NOT: struct Payload

// A single entry point is named on the downstream command line, so it needs no attribute,
// and a pre-SM6.1 profile would reject one.
// SM51-NOT: [shader(

// A shader model the user asked for explicitly stays authoritative, so a whole-program
// compile pinned to SM 5.1 keeps emitting output that SM 5.1 can accept. Only the default
// profile lets the whole-program option decide.
// WHOLESM51-NOT: [shader(

RWStructuredBuffer<uint> Output;

struct Payload
{
    uint hit;
};

uint sharedHelper(uint a)
{
    return a + 3U;
}

[shader("compute")]
[numthreads(64, 1, 1)]
void MainCS(uint3 tid: SV_DispatchThreadID)
{
    Output[tid.x] = sharedHelper(tid.x);
}

[shader("closesthit")]
void MainCH(inout Payload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.hit = sharedHelper(1U);
}

[shader("anyhit")]
void MainAH(inout Payload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.hit = sharedHelper(2U);
}
