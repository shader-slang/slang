//TEST:SIMPLE(filecheck=LINEAR_SPV): -target spirv-asm -DX=16 -DY=1 -DZ=1
//TEST:SIMPLE(filecheck=LINEAR_GLSL): -target glsl -DX=16 -DY=1 -DZ=1
//TEST:SIMPLE(filecheck=LINEAR_SPV): -target spirv-asm -DX=2 -DY=1 -DZ=2
//TEST:SIMPLE(filecheck=LINEAR_GLSL): -target glsl -DX=2 -DY=1 -DZ=2
//TEST:SIMPLE(filecheck=LINEAR_SPV): -target spirv-asm -DX=16 -DY=foldedY -DZ=1
//TEST:SIMPLE(filecheck=LINEAR_GLSL): -target glsl -DX=16 -DY=foldedY -DZ=1
//TEST:SIMPLE(filecheck=QUAD_SPV): -target spirv-asm -DX=2 -DY=2 -DZ=1
//TEST:SIMPLE(filecheck=QUAD_GLSL): -target glsl -DX=2 -DY=2 -DZ=1
//TEST:SIMPLE(filecheck=LINEAR_SPV): -target spirv-asm -DX=2 -DY=2 -DZ=1 -DEXPLICIT_LINEAR
//TEST:SIMPLE(filecheck=LINEAR_GLSL): -target glsl -DX=2 -DY=2 -DZ=1 -DEXPLICIT_LINEAR
//TEST:SIMPLE(filecheck=QUAD_SPV): -target spirv-asm -DX=2 -DY=specY -DZ=1
//TEST:SIMPLE(filecheck=QUAD_GLSL): -target glsl -DX=2 -DY=specY -DZ=1
//TEST:SIMPLE(filecheck=NONE_SPV): -target spirv-asm -DX=3 -DY=1 -DZ=1 -DNO_DERIVATIVES
//TEST:SIMPLE(filecheck=NONE_GLSL): -target glsl -DX=3 -DY=1 -DZ=1 -DNO_DERIVATIVES

//TEST:SIMPLE(filecheck=LINEAR_SPV): -target spirv-asm -DX=specX -DY=1 -DZ=1
//TEST:SIMPLE(filecheck=LINEAR_GLSL): -target glsl -DX=specX -DY=1 -DZ=1
//TEST:SIMPLE(filecheck=QUAD_SPV): -target spirv-asm -DX=2 -DY=2 -DZ=1 -DEXPLICIT_QUAD
//TEST:SIMPLE(filecheck=QUAD_GLSL): -target glsl -DX=2 -DY=2 -DZ=1 -DEXPLICIT_QUAD
//TEST:SIMPLE(filecheck=LINEAR_SPV): -target spirv-asm -emit-spirv-via-glsl -DX=16 -DY=1 -DZ=1

// LINEAR_SPV: OpCapability ComputeDerivativeGroupLinearKHR
// LINEAR_SPV: OpExecutionMode %main DerivativeGroupLinearKHR
// LINEAR_SPV-NOT: DerivativeGroupQuads
// QUAD_SPV: OpCapability ComputeDerivativeGroupQuadsKHR
// QUAD_SPV: OpExecutionMode %main DerivativeGroupQuadsKHR
// QUAD_SPV-NOT: DerivativeGroupLinear
// LINEAR_GLSL-NOT: derivative_group_quadsNV
// LINEAR_GLSL: layout(derivative_group_linearNV) in;
// LINEAR_GLSL-NOT: derivative_group_
// QUAD_GLSL-NOT: derivative_group_linearNV
// QUAD_GLSL: layout(derivative_group_quadsNV) in;
// QUAD_GLSL-NOT: derivative_group_
// NONE_SPV-NOT: DerivativeGroup
// NONE_SPV: OpFunctionEnd
// NONE_GLSL-NOT: derivative_group_
// NONE_GLSL: void main()
// NONE_GLSL-NOT: derivative_group_

Texture2D<float4> texture;
SamplerState sampler;
RWStructuredBuffer<float4> output;

[vk::constant_id(1)]
const int specX = 16;

static const int foldedY = 2 - 1;
[vk::constant_id(0)]
const int specY = 2;

[shader("compute")]
[numthreads(X, Y, Z)]
#ifdef EXPLICIT_LINEAR
[DerivativeGroupLinear]
#endif
#ifdef EXPLICIT_QUAD
[DerivativeGroupQuad]
#endif
void main(uint3 tid : SV_DispatchThreadID)
{
#ifdef NO_DERIVATIVES
    output[tid.x] = texture.SampleLevel(sampler, float2(tid.xy), 0);
#else
    output[tid.x] = texture.Sample(sampler, float2(tid.xy))
        + texture.Sample(sampler, float2(tid.yx));
#endif
}
