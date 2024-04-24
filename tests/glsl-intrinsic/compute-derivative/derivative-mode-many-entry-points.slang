//TEST:SIMPLE(filecheck=CHECK_SPV_SYNTAX): -allow-glsl -fvk-use-entrypoint-name -target spirv -emit-spirv-directly
// CHECK_SPV_SYNTAX: DerivativeGroupQuadsNV
// CHECK_SPV_SYNTAX: DerivativeGroupLinearNV

//TEST:SIMPLE(filecheck=CHECK_GLSL_SYNTAX): -allow-glsl -fvk-use-entrypoint-name -target spirv -emit-spirv-directly -DGLSL_SYNTAX
// CHECK_GLSL_SYNTAX: DerivativeGroupQuadsNV
// CHECK_GLSL_SYNTAX: DerivativeGroupQuadsNV
#ifdef GLSL_SYNTAX
layout(derivative_group_quadsNV) in;
#endif

buffer MyBlockName
{
    int result;
} outputBuffer;

[DerivativeGroupQuad]
[shader("compute")]
[numthreads(2,2,1)]
void main1(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer.result = asint(ddx(1));
}

[DerivativeGroupLinear]
[shader("compute")]
void main2(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer.result = asint(ddx(1));
}
