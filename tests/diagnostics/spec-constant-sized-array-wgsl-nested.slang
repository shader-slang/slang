// WGSL permits an override-sized array in workgroup storage only as the outermost array type
// (#14030).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute

[SpecializationConstant]
const int kTile = 4;

groupshared float allowed[kTile];

groupshared float bothSized[kTile][kTile];
//CHECK:          ^^^^^^^^^ error E55222
//CHECK:          ^^^^^^^^^ cannot size an array with a specialization constant

groupshared float innerSized[4][kTile];
//CHECK:          ^^^^^^^^^^ error E55222
//CHECK:          ^^^^^^^^^^ cannot size an array with a specialization constant

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint gi: SV_GroupIndex)
{
    allowed[gi] = gi;
    bothSized[gi][gi] = gi;
    innerSized[gi][gi] = gi;
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[gi] = allowed[gi] + bothSized[gi][gi] + innerSized[gi][gi];
}
