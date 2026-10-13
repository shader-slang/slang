// post-depth-coverage-unsupported-target.slang
//
// `[postdepthcoverage]` lowers to an execution mode only on the Khronos targets (SPIR-V / GLSL).
// On any other target the decoration is silently dropped, which would leave a shader that reads
// `SV_Coverage` running with ordinary pre-depth coverage — a different result with no compile-time
// signal. The compiler must warn rather than silently ignore it (shader-slang/slang#12999).
//
// Here `-target hlsl` (a non-Khronos target) must emit the warning; `-target spirv` (supported)
// must not.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry main -stage fragment

// HLSL: warning{{.*}}50062
// HLSL-SAME: postdepthcoverage
// The detailed message names the offending target (verifies the ~target interpolation).
// HLSL: target 'hlsl'

// SPIRV-NOT: 50062

[postdepthcoverage]
[shader("fragment")]
float4 main(uint coverage : SV_Coverage) : SV_Target
{
    return float4((float)coverage, 0, 0, 1);
}
