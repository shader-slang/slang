//TEST:SIMPLE_EX(filecheck=CHECK_SPV_SYNTAX): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-many-entry-points1.slang -fvk-use-entrypoint-name -target spirv -Wno-118
// CHECK_SPV_SYNTAX: DerivativeGroupQuads{{NV|KHR}}
// CHECK_SPV_SYNTAX: DerivativeGroupLinear{{NV|KHR}}

//TEST:SIMPLE_EX(filecheck=CHECK_GLSL_SYNTAX): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-many-entry-points1.slang -fvk-use-entrypoint-name -target spirv -DGLSL_SYNTAX -Wno-118
// CHECK_GLSL_SYNTAX: DerivativeGroupQuads{{NV|KHR}}
// CHECK_GLSL_SYNTAX: DerivativeGroupQuads{{NV|KHR}}
#ifdef GLSL_SYNTAX
layout(derivative_group_quadsNV) in;
#endif

buffer MyBlockName
{
    int result;
} outputBuffer;

#if !defined(GLSL_SYNTAX)
[DerivativeGroupQuad]
#endif
[shader("compute")]
[numthreads(2,2,1)]
void main1(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer.result = asint(ddx(1));
}

#if !defined(GLSL_SYNTAX)
[DerivativeGroupLinear]
[shader("compute")]
[numthreads(4,1,1)]
void main2(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer.result = asint(ddx(1));
}
#endif
