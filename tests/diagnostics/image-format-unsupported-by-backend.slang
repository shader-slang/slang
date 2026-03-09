//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_WGSL): -target wgsl

// CHECK_SPIRV: warning{{.*}}Image format 'bgra8' is not explicitly supported by the SPIRV backend
// CHECK_GLSL: warning{{.*}}Image format 'bgra8' is not explicitly supported by the GLSL backend
[format("bgra8")]
RWTexture2D<float4> outputTexture;

// CHECK_WGSL: warning{{.*}}Image format 'rg8' is not explicitly supported by the WGSL backend
[format("rg8")]
RWTexture2D<float4> outputTexture2;

[numthreads(8, 8, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    outputTexture[threadID.xy] = float4(1.0, 1.0, 1.0, 1.0);
    outputTexture2[threadID.xy] = float4(1.0, 1.0, 1.0, 1.0);
}
