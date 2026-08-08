// Requesting a SPIRV capability on an HLSL target should error.
//TEST:SIMPLE(filecheck=CHECK_ERR): -target hlsl -entry main -stage compute -capability spirv_1_5
// CHECK_ERR: error[E36121]
// CHECK_ERR: spirv_1_5

// The same SPIRV capability on a SPIRV target (direct emit, the default) is fine:
// spirv_1_5 is genuinely compatible with a spirv target, so isIncompatibleWith returns false.
//TEST:SIMPLE(filecheck=CHECK_OK): -target spirv -entry main -stage compute -capability spirv_1_5
// CHECK_OK: OpEntryPoint
// CHECK_OK-NOT: E36121

// SPIRV capability on a SPIRV target forced through the GLSL-SPIRV pipeline: the SPIRV
// cap is auto-converted to a glsl_spirv_* equivalent by getTargetCaps(), so no error.
// This exercises the isGLSLBasedTarget() + SPIRV-cap exemption branch.
//TEST:SIMPLE(filecheck=CHECK_VIA_GLSL_OK): -target spirv -emit-spirv-via-glsl -entry main -stage compute -capability spirv_1_5
// CHECK_VIA_GLSL_OK-NOT: E36121

// -ignore-capabilities suppresses E36121.
//TEST:SIMPLE(filecheck=CHECK_IGNORE): -target hlsl -entry main -stage compute -capability spirv_1_5 -ignore-capabilities
// CHECK_IGNORE: result code = 0
// CHECK_IGNORE-NOT: E36121

// GLSL target with a SPIRV capability: auto-converted to glsl_spirv_*, no error.
//TEST:SIMPLE(filecheck=CHECK_GLSL_OK): -target glsl -entry main -stage compute -capability spirv_1_5
// CHECK_GLSL_OK: result code = 0
// CHECK_GLSL_OK-NOT: E36121

// Requesting a GLSL feature capability on a CPP target should error.
//TEST:SIMPLE(filecheck=CHECK_ERR_CPP): -target cpp -entry main -stage compute -capability image_loadstore
// CHECK_ERR_CPP: error[E36121]
// CHECK_ERR_CPP: image_loadstore

// Requesting an HLSL capability on a GLSL target should also error.
//TEST:SIMPLE(filecheck=CHECK_ERR_HLSL_ON_GLSL): -target glsl -entry main -stage compute -capability _sm_6_6
// CHECK_ERR_HLSL_ON_GLSL: error[E36121]
// CHECK_ERR_HLSL_ON_GLSL: _sm_6_6

// Metal target with an HLSL capability should error, and the diagnostic must say 'metal' not a derived atom.
//TEST:SIMPLE(filecheck=CHECK_ERR_METAL): -target metal -entry main -stage compute -capability _sm_6_6
// CHECK_ERR_METAL: error[E36121]
// CHECK_ERR_METAL: _sm_6_6
// CHECK_ERR_METAL: metal

// WGSL target with a SPIRV capability should error.
//TEST:SIMPLE(filecheck=CHECK_ERR_WGSL): -target wgsl -entry main -stage compute -capability spirv_1_5
// CHECK_ERR_WGSL: error[E36121]
// CHECK_ERR_WGSL: spirv_1_5
// CHECK_ERR_WGSL: wgsl

// CUDA target with a GLSL capability should error.
//TEST:SIMPLE(filecheck=CHECK_ERR_CUDA): -target cuda -entry main -stage compute -capability _GLSL_460
// CHECK_ERR_CUDA: error[E36121]
// CHECK_ERR_CUDA: _GLSL_460
// CHECK_ERR_CUDA: cuda

// Direct SPIRV target with a SPIRV capability valid for that target: no error.
//TEST:SIMPLE(filecheck=CHECK_SPIRV_DIRECT_OK): -target spirv -emit-spirv-directly -entry main -stage compute -capability spirv_1_5
// CHECK_SPIRV_DIRECT_OK: OpEntryPoint
// CHECK_SPIRV_DIRECT_OK-NOT: E36121

// Note: cross-target version aliases like sm_6_0 intentionally include SPIRV paths
// (sm_6_0 maps to spirv_1_3 for SPIRV targets), so they do NOT trigger E36121.

[numthreads(1, 1, 1)]
void main() {}
