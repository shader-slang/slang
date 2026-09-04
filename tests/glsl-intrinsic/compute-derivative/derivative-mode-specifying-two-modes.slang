//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-specifying-two-modes.slang -stage compute -entry computeMain -target glsl -DSLANG -Wno-118
//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-specifying-two-modes.slang -stage compute -entry computeMain -target spirv -DSLANG -Wno-118

//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-specifying-two-modes.slang -stage compute -entry computeMain -target glsl -DGLSL -Wno-118
//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-specifying-two-modes.slang -stage compute -entry computeMain -target spirv -DGLSL -Wno-118
// CHECK: error[E31212]

buffer MyBlockName
{
    int result;
} outputBuffer;

#ifdef SLANG
[DerivativeGroupQuad]
[DerivativeGroupLinear]
#endif
#ifdef GLSL
layout(derivative_group_quadsNV) in;
layout(derivative_group_linearNV) in;
#endif
[numthreads(2, 2, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer.result = asint(ddx(1));
}
