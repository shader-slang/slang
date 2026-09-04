//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-wrong-multiple-quad.slang -stage compute -entry computeMain -target glsl -DQUAD_SLANG -Wno-118
//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-wrong-multiple-quad.slang -stage compute -entry computeMain -target spirv -DQUAD_SLANG -Wno-118

//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-wrong-multiple-quad.slang -stage compute -entry computeMain -target glsl -DQUAD_GLSL -Wno-118
//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-wrong-multiple-quad.slang -stage compute -entry computeMain -target spirv -DQUAD_GLSL -Wno-118
// CHECK: error[E31210]

buffer MyBlockName
{
    int result;
} outputBuffer;

#ifdef QUAD_SLANG
[DerivativeGroupQuad]
[numthreads(3, 3, 1)]
#endif
#ifdef QUAD_GLSL
layout(derivative_group_quadsNV) in;
[numthreads(3, 3, 1)]
#endif
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer.result = asint(ddx(1));
}
