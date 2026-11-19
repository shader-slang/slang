// Regression test for shader-slang/slang#12785 (a `volatile` qualifier on a read-only resource
// defeats the repeatable-read gate — with a plain control that isolates the qualifier).
//
// A `StructuredBuffer` is a read-only resource TYPE, but a `volatile` qualifier on it means each
// read must be re-performed, so a `[noinline]` wrapper around a volatile read is not a repeatable
// read-only access and its two dominated identical calls must not be commoned. The plain
// (unqualified) sibling IS commoned, so the qualifier -- not the resource type -- is what blocks:
// the qualifier is an `IRMemoryQualifierSetDecoration` on the resource that `isRepeatableReadLocation`
// inspects (the type check alone, `isPointerToImmutableLocation`, would accept it).

//TEST:SIMPLE(filecheck=VOLATILE): -target hlsl -entry mainVolatile -stage compute
//TEST:SIMPLE(filecheck=PLAIN): -target hlsl -entry mainPlain -stage compute

volatile StructuredBuffer<uint> volData;
StructuredBuffer<uint> plainData;
RWStructuredBuffer<uint> outputData;

[noinline]
uint loadVolatile(uint index)
{
    return volData[index];
}

[noinline]
uint loadPlain(uint index)
{
    return plainData[index];
}

// volatile SRV: not repeatable, so both calls survive.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainVolatile(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    outputData[i] = loadVolatile(i) + loadVolatile(i);
}
// VOLATILE-COUNT-2: loadVolatile_0(i
// VOLATILE-NOT: loadVolatile_0(i

// plain SRV control: repeatable, so the two calls are commoned to one.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainPlain(uint3 tid : SV_DispatchThreadID)
{
    uint i = tid.x;
    outputData[i] = loadPlain(i) + loadPlain(i);
}
// PLAIN-COUNT-1: loadPlain_0(i
// PLAIN-NOT: loadPlain_0(i
