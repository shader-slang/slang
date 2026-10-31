//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK_WGSL): -target wgsl -DTARGET_WGSL

// CHECK_SPIRV: warning{{.*}}Image format 'bgra8' is not explicitly supported by the SPIRV backend
// CHECK_GLSL: warning{{.*}}Image format 'bgra8' is not explicitly supported by the GLSL backend
[format("bgra8")]
RWTexture2D<float4> outputTexture;

// WGSL has no 64-bit texel format. The declaration is WGSL-only because `r64ui` on a
// float image is invalid SPIR-V.
#ifdef TARGET_WGSL
// CHECK_WGSL: warning{{.*}}Image format 'r64ui' is not explicitly supported by the WGSL backend
[format("r64ui")]
RWTexture2D<float4> outputTexture2;
#endif

[numthreads(8, 8, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    outputTexture[threadID.xy] = float4(1.0, 1.0, 1.0, 1.0);
#ifdef TARGET_WGSL
    outputTexture2[threadID.xy] = float4(1.0, 1.0, 1.0, 1.0);
#endif
}
