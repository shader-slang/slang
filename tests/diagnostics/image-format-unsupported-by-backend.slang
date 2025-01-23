//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_SPIRV): -target glsl

// CHECK_SPIRV: warning 31105
// CHECK_GLSL: warning 31105
[format("bgra8")]
RWTexture2D<float4> outputTexture;

[numthreads(8, 8, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    outputTexture[threadID.xy] = float4(1.0, 1.0, 1.0, 1.0);
}
