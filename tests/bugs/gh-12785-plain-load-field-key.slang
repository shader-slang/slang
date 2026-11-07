// Regression test for shader-slang/slang#12785 (plain-`Load` branch checks the field-key qualifier).
//
// A coherent scalar member of a `ConstantBuffer` is read as a plain `load(fieldAddr(cb, xKey))`. On
// its own such a wrapper is `readNone` (a ConstantBuffer read has no OOB hazard), so it is commoned
// by the movable path and never reaches this tier. To force the tier to be consulted, the wrapper
// here ALSO reads a plain `StructuredBuffer` -- which denies `readNone` (gh-3429) -- so eligibility
// runs through `calleeIsRepeatableReadOnly`, and the coherent ConstantBuffer member is checked by
// the plain-`Load` branch. That branch hands the raw field pointer to `isRepeatableReadLocation`
// (rather than pre-stripping with `getRootAddr`, which would drop the field key), so the coherent
// member is seen and the two dominated calls survive. The plain control uses an unqualified member
// and commons.

//TEST:SIMPLE(filecheck=COHERENT): -target hlsl -entry mainCoherent -stage compute
//TEST:SIMPLE(filecheck=PLAIN): -target hlsl -entry mainPlain -stage compute

struct SC
{
    globallycoherent uint x;
    uint y;
}
struct SP
{
    uint x;
    uint y;
}
ConstantBuffer<SC> cbC;
ConstantBuffer<SP> cbP;
StructuredBuffer<uint> ro; // read to deny readNone and force the tier to be consulted
RWStructuredBuffer<uint> outputData;

[noinline]
uint readCoherent(uint index)
{
    return ro[index] + cbC.x; // plain StructuredBuffer read + coherent CB scalar member (plain Load)
}

[noinline]
uint readPlain(uint index)
{
    return ro[index] + cbP.x; // plain StructuredBuffer read + unqualified CB scalar member
}

// Coherent member reached via the plain-`Load` branch: not repeatable, so both calls survive.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainCoherent(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    outputData[i] = readCoherent(i) + readCoherent(i);
}
// COHERENT-COUNT-2: readCoherent_0(i
// COHERENT-NOT: readCoherent_0(i

// Unqualified member: repeatable, so the two dominated calls are commoned to one.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainPlain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    outputData[i] = readPlain(i) + readPlain(i);
}
// PLAIN-COUNT-1: readPlain_0(i
// PLAIN-NOT: readPlain_0(i
