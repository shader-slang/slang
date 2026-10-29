// WGSL allows an override-sized array only as the store type of a `var<workgroup>`, so a
// function that returns a copy of one is reported at the call (#14030).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute

[SpecializationConstant]
const int kTile = 4;

groupshared float tile[kTile];

RWStructuredBuffer<float> outputBuffer;

[noinline]
float[kTile] snapshot()
{
    return tile;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID)
{
    tile[tid] = tid;
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[tid] = snapshot()[tid];
//CHECK:                        ^ error E55222
//CHECK:                        ^ cannot size an array with a specialization constant
}
