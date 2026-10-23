// Regression test for shader-slang/slang#12785 (coherent/volatile qualifier on a read-only-typed
// resource is not a repeatable read).
//
// A `globallycoherent` (or `volatile`) qualifier can sit on an otherwise read-only resource such as
// `StructuredBuffer`. Its resource *type* is immutable, but the qualifier means each read must be
// re-performed, so a wrapper around it is not a repeatable read-only access and its calls must not
// be commoned. The qualifier is an `IRMemoryQualifierSetDecoration` on the resource, separate from
// the type, so the gate must inspect it (isPointerToImmutableLocation checks only the type).

//TEST:SIMPLE(filecheck=SCALAR): -target hlsl -entry mainScalar -stage compute
//TEST:SIMPLE(filecheck=ARRAY): -target hlsl -entry mainArray -stage compute

globallycoherent StructuredBuffer<uint> inputData;
volatile StructuredBuffer<uint> inputArray[2];
RWStructuredBuffer<uint> outputData;

[noinline]
uint loadValue(uint index)
{
    return inputData[index];
}

// The qualifier can also sit on an aggregate (array/struct) of buffers accessed by value/element,
// so the gate must peel value-form access (`getElement`/`fieldExtract`) and field keys to reach it.
[noinline]
uint loadArrayValue(uint index)
{
    return inputArray[0][index];
}

// globallycoherent scalar SRV: not repeatable, calls stay distinct.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainScalar(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    uint a = loadValue(index);
    uint b = loadValue(index);
    outputData[index] = a + b;
}
// SCALAR-COUNT-2: loadValue_0(index
// SCALAR-NOT: loadValue_0(index

// volatile SRV reached through an array element (value-form access): still not repeatable.
[shader("compute")]
[numthreads(1, 1, 1)]
void mainArray(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    uint a = loadArrayValue(index);
    uint b = loadArrayValue(index);
    outputData[index] = a + b;
}
// ARRAY-COUNT-2: loadArrayValue_0(index
// ARRAY-NOT: loadArrayValue_0(index
