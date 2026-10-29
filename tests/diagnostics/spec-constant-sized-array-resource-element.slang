// An array sized by a specialization constant as a buffer element type, as the value a buffer
// load returns, or as a global uniform is reported at the declaration or the load on targets
// that cannot size an array that way (#14030).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute -warnings-disable 39019
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute

[SpecializationConstant]
const int kTile = 4;

RWStructuredBuffer<float[kTile]> tiles;
//CHECK:                         ^^^^^ error E55222
//CHECK:                         ^^^^^ cannot size an array with a specialization constant

ByteAddressBuffer input;

uniform int weights[kTile];
//CHECK:    ^^^^^^^ error E55222
//CHECK:    ^^^^^^^ cannot size an array with a specialization constant

RWStructuredBuffer<float2[kTile]> pairs;
//CHECK:                          ^^^^^ error E55222
//CHECK:                          ^^^^^ cannot size an array with a specialization constant

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID)
{
    tiles[0][tid % kTile] = input.Load<float[kTile]>(0)[tid % kTile];
//CHECK:                                            ^ error E55222
//CHECK:                                            ^ cannot size an array with a specialization constant
    pairs[0][tid % kTile] = input.Load<float2[kTile]>(16)[tid % kTile] * float(weights[tid % kTile]);
//CHECK:                                             ^ error E55222
//CHECK:                                             ^ cannot size an array with a specialization constant
}
