//TEST:SIMPLE(filecheck=CHECK_SPV_SYNTAX): -allow-glsl -stage compute -entry main -target spirv -emit-spirv-directly
// CHECK_SPV_SYNTAX: DerivativeGroupQuadsNV

//TEST:SIMPLE(filecheck=CHECK_GLSL_SYNTAX): -allow-glsl -stage compute -entry main -target glsl
// CHECK_GLSL_SYNTAX: derivative_group_quadsNV

buffer MyBlockName
{
    int result;
} outputBuffer;

int someFunc()
{
    return asint(ddx(1));
}

void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer.result = someFunc();
}