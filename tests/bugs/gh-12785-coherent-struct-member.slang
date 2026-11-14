// Regression test for shader-slang/slang#12785 (coherent/volatile member inside a loaded aggregate).
//
// A `globallycoherent` qualifier can sit on a struct MEMBER of a `StructuredBuffer` element. Reading
// `buf[i].x` lowers to `getField(structuredBufferLoad(buf, i), xKey)`: the whole element struct is
// loaded (a resource load) and the member extracted by value. The buffer handle itself is not
// qualified -- the coherent-ness lives on the member's field key / type -- so a location-based check
// (`isRepeatableReadLocation`, which vets the buffer handle's provenance and access-chain keys) does
// not see it, and the by-value `getField` carries no location provenance either. Without the
// loaded-type check the two dominated calls would be wrongly commoned, dropping a required coherent
// re-read. `typeContainsNonRepeatableQualifiedMember(loadedType)` rejects the read, so both calls
// survive.
//
// `mainNested` pins that the loaded-type walk recurses: the coherent member is nested one struct
// deeper (an outer field whose own key is unqualified), so only a transitive walk rejects it. The
// plain buffer proves the check does not over-reject: an unqualified element type still commons.

//TEST:SIMPLE(filecheck=COHERENT): -target hlsl -entry mainCoherent -stage compute
//TEST:SIMPLE(filecheck=NESTED): -target hlsl -entry mainNested -stage compute
//TEST:SIMPLE(filecheck=PLAIN): -target hlsl -entry mainPlain -stage compute

struct Coh
{
    globallycoherent uint x; // coherent member -> element load is not repeatable
    uint y;
}
struct Plain
{
    uint x;
    uint y;
}
struct Outer
{
    Coh inner; // outer key is unqualified; the coherent member is one level deeper
    uint z;
}

StructuredBuffer<Coh> bufCoh;
StructuredBuffer<Outer> bufNested;
StructuredBuffer<Plain> bufPlain;
RWStructuredBuffer<uint> outputData;

[noinline]
uint loadCoh(uint index)
{
    return bufCoh[index].x;
}

[noinline]
uint loadNested(uint index)
{
    return bufNested[index].inner.x;
}

[noinline]
uint loadPlain(uint index)
{
    return bufPlain[index].x;
}

// Coherent member: the element type carries a coherent member, so the load is not repeatable and the
// two dominated calls survive. Anchor on the call (arg `i`); the definition `loadCoh_0(uint index_0)`
// does not match `loadCoh_0(i`.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainCoherent(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    outputData[i] = loadCoh(i) + loadCoh(i);
}
// COHERENT-COUNT-2: loadCoh_0(i
// COHERENT-NOT: loadCoh_0(i

// Nested coherent member: only a transitive type walk finds the coherent member one struct deeper.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainNested(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    outputData[i] = loadNested(i) + loadNested(i);
}
// NESTED-COUNT-2: loadNested_0(i
// NESTED-NOT: loadNested_0(i

// Plain element type: repeatable, so the two dominated calls are commoned to one.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainPlain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    outputData[i] = loadPlain(i) + loadPlain(i);
}
// PLAIN-COUNT-1: loadPlain_0(i
// PLAIN-NOT: loadPlain_0(i
