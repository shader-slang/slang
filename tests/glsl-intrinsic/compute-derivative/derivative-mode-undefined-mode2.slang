//TEST:SIMPLE_EX(filecheck=CHECK_SPV_SYNTAX): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-undefined-mode2.slang -stage compute -entry main -target spirv -Wno-118
// CHECK_SPV_SYNTAX: DerivativeGroupQuads{{NV|KHR}}

//TEST:SIMPLE_EX(filecheck=CHECK_GLSL_SYNTAX): -lang glsl tests/glsl-intrinsic/compute-derivative/derivative-mode-undefined-mode2.slang -stage compute -entry main -target glsl -Wno-118
// CHECK_GLSL_SYNTAX: derivative_group_quadsNV

buffer MyBlockName
{
    int result;
} outputBuffer;

int someFunc()
{
    return asint(ddx(1));
}

[numthreads(2, 2, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer.result = someFunc();
}
