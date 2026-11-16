//DIAGNOSTIC_TEST:SIMPLE(diag=LINEAR): -target spirv-asm -DX=3 -DY=1 -DTEST_LINEAR
//DIAGNOSTIC_TEST:SIMPLE(diag=LINEAR_GLSL): -target glsl -DX=3 -DY=1 -DTEST_LINEAR
//DIAGNOSTIC_TEST:SIMPLE(diag=EXPLICIT): -target spirv-asm -DX=16 -DY=1 -DEXPLICIT_QUAD
//DIAGNOSTIC_TEST:SIMPLE(diag=EXPLICIT): -target glsl -DX=16 -DY=1 -DEXPLICIT_QUAD
//DIAGNOSTIC_TEST:SIMPLE(diag=QUAD): -target spirv-asm -DX=16 -DY=specY
//DIAGNOSTIC_TEST:SIMPLE(diag=QUAD_GLSL): -target glsl -DX=16 -DY=specY
//DIAGNOSTIC_TEST:SIMPLE(diag=QUAD): -target spirv-asm -DX=4 -DY=3
//DIAGNOSTIC_TEST:SIMPLE(diag=QUAD_GLSL): -target glsl -DX=4 -DY=3

//LINEAR_GLSL: E31211
//LINEAR_GLSL: compute derivative group linear requires total thread dispatch count to be at a multiple of 4
//QUAD_GLSL: E31210
//QUAD_GLSL: compute derivative group quad requires thread dispatch count of X and Y to each be at a multiple of 2

Texture2D<float4> texture;
SamplerState sampler;
RWStructuredBuffer<float4> output;

// A specialization constant with default 1 must still infer quad grouping.
[vk::constant_id(0)]
const int specY = 1;

[shader("compute")]
[numthreads(X, Y, 1)]
#ifdef EXPLICIT_QUAD
[DerivativeGroupQuad]
#endif
void main(uint3 tid : SV_DispatchThreadID)
/*EXPLICIT:
     ^^^^ E31210
     ^^^^ compute derivative group quad requires thread dispatch count of X and Y to each be at a multiple of 2
*/
{
#ifdef TEST_LINEAR
    output[tid.x] = texture.Sample(sampler, float2(tid.xy));
//LINEAR:                         ^ E31211
//LINEAR:                         ^ compute derivative group linear requires total thread dispatch count to be at a multiple of 4
#else
    output[tid.x] = texture.Sample(sampler, float2(tid.xy));
//QUAD:                           ^ E31210
//QUAD:                           ^ compute derivative group quad requires thread dispatch count of X and Y to each be at a multiple of 2
#endif
}
