// The plain-offset combined-sampler Sample lowers through core textureOffset on GLSL and a
// ConstOffset-only implicit-lod sample on SPIR-V: only the (offset, clamp) overloads may use
// the ARB clamp intrinsics or the MinLod operand.
//TEST:SIMPLE(filecheck=GLSL):-target glsl -profile glsl_460 -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage fragment -entry fragmentMain

Sampler2D combined2D;

float4 fragmentMain(float4 p: SV_Position) : SV_Target
{
    return combined2D.Sample(p.xy, int2(1, 0));
}

// GLSL: #version
// GLSL-NOT: GL_ARB_sparse_texture_clamp
// GLSL: textureOffset(
// SPV: OpImageSampleImplicitLod
// SPV-NOT: MinLod
