//DIAGNOSTIC_TEST:SIMPLE_EX(diag=SLANG): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-wrong-multiple-linear.slang -stage compute -entry computeMain -target glsl -DLINEAR_SLANG -Wno-118
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=SLANG): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-wrong-multiple-linear.slang -stage compute -entry computeMain -target spirv -DLINEAR_SLANG -Wno-118

//DIAGNOSTIC_TEST:SIMPLE_EX(diag=GLSL): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-wrong-multiple-linear.slang -stage compute -entry computeMain -target glsl -DLINEAR_GLSL -Wno-118
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=GLSL): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-wrong-multiple-linear.slang -stage compute -entry computeMain -target spirv -DLINEAR_GLSL -Wno-118

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
//GLSL:                             ^ derivative group linear thread count error
//GLSL:                             ^ compute derivative group linear requires total thread dispatch count to be at a multiple of 4
[numthreads(1, 3, 1)]
#endif

void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
/*SLANG:
     ^^^^^^^^^^^ error E31211
     ^^^^^^^^^^^ compute derivative group linear requires total thread dispatch count to be at a multiple of 4
*/
{
    outputBuffer.result = asint(ddx(1));
}
