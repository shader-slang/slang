//TEST:SIMPLE_EX(filecheck=CHECK_SPV_SYNTAX): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-undefined-mode.slang -fvk-use-entrypoint-name -target spirv -Wno-118
// CHECK_SPV_SYNTAX: DerivativeGroupQuads{{NV|KHR}}
// CHECK_SPV_SYNTAX: DerivativeGroupQuads{{NV|KHR}}

//TEST:SIMPLE_EX(filecheck=CHECK_GLSL_SYNTAX): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-undefined-mode.slang -stage compute -entry main2 -target glsl -Wno-118
// CHECK_GLSL_SYNTAX: derivative_group_quadsNV

buffer MyBlockName
{
    int result;
} outputBuffer;

[shader("compute")]
[numthreads(2, 2, 1)]
void main1(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer.result = asint(ddx(1));
}

[shader("compute")]
[numthreads(2, 2, 1)]
void main2(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer.result = asint(ddx(1));
}
