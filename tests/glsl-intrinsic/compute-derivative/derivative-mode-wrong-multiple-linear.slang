//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-wrong-multiple-linear.slang -stage compute -entry computeMain -target glsl -DLINEAR_SLANG -Wno-118
//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-wrong-multiple-linear.slang -stage compute -entry computeMain -target spirv -DLINEAR_SLANG -Wno-118

//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-wrong-multiple-linear.slang -stage compute -entry computeMain -target glsl -DLINEAR_GLSL -Wno-118
//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-wrong-multiple-linear.slang -stage compute -entry computeMain -target spirv -DLINEAR_GLSL -Wno-118
// CHECK: error[E31211]

buffer MyBlockName
{
    int result;
} outputBuffer;

#ifdef LINEAR_SLANG
[DerivativeGroupLinear]
[numthreads(1, 3, 1)]
#endif

#ifdef LINEAR_GLSL
layout(derivative_group_linearNV) in;
[numthreads(1, 3, 1)]
#endif

void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer.result = asint(ddx(1));
}
