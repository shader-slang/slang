// WGSL accepts a `var<workgroup>` sized by a specialization constant, but not a list assigned to
// it (#14030).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute

[SpecializationConstant]
const int kTile = 4;

groupshared float tile[kTile];

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID)
{
    tile = {};
//CHECK:   ^ error E55223
//CHECK:   ^ initialized from a list
    outputBuffer[0] = tile[tid % kTile];
}
