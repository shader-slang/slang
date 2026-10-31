// Regression test for shader-slang/slang#12785 (per-member coherent qualifier on an immutable
// aggregate — proves the field-key qualifier check in isRepeatableReadLocation is reachable).
//
// A `ConstantBuffer<Bundle>` whose members are resources lowers each member read to
// `structuredBufferLoad(load(fieldAddr(bundle, memberKey)), i)`. `getRootAddr` at the call site does
// NOT peel a load, so the handle-`load` is handed to `isRepeatableReadLocation`, whose own peel steps
// through the `Load` to the `FieldAddress` and inspects the member field key's
// `globallycoherent`/`volatile` qualifier. So a `globallycoherent` member must NOT be commoned, while
// a plain member of the same immutable aggregate is repeatable and IS commoned. This pins that the
// field-key check fires (it is not dead) and that it discriminates per member.

//TEST:SIMPLE(filecheck=PLAIN): -target hlsl -entry mainPlain -stage compute
//TEST:SIMPLE(filecheck=COHERENT): -target hlsl -entry mainCoherent -stage compute

struct Bundle
{
    StructuredBuffer<uint> a;               // plain -> repeatable
    globallycoherent StructuredBuffer<uint> b; // coherent -> NOT repeatable
}
ConstantBuffer<Bundle> bundle;
RWStructuredBuffer<uint> outputData;

[noinline]
uint loadA(uint i)
{
    return bundle.a[i];
}

[noinline]
uint loadB(uint i)
{
    return bundle.b[i];
}

// Plain member: the two dominated identical calls are commoned to one.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainPlain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    outputData[i] = loadA(i) + loadA(i);
}
// PLAIN-COUNT-1: loadA_0(i
// PLAIN-NOT: loadA_0(i

// globallycoherent member: the coherent field key defeats repeatability, so both calls survive.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainCoherent(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    outputData[i] = loadB(i) + loadB(i);
}
// COHERENT-COUNT-2: loadB_0(i
// COHERENT-NOT: loadB_0(i
