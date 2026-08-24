//TEST:SIMPLE(filecheck=CHECK_EXT_ERR): -target glsl -entry main -stage compute -capability SPV_KHR_ray_tracing
// CHECK_EXT_ERR: error[E36121]
// CHECK_EXT_ERR: SPV_KHR_ray_tracing

// The GLSL-SPIRV pipeline (-emit-spirv-via-glsl) hits the same isGLSLBasedTarget() exemption, so
// the extension must be diagnosed there too.
//TEST:SIMPLE(filecheck=CHECK_VIA_GLSL_EXT_ERR): -target spirv -emit-spirv-via-glsl -entry main -stage compute -capability SPV_KHR_ray_tracing
// CHECK_VIA_GLSL_EXT_ERR: error[E36121]
// CHECK_VIA_GLSL_EXT_ERR: SPV_KHR_ray_tracing

// spirv_1_5's expansion also carries SPIRV extension atoms, so this pins that the exemption keys on
// the requested version, not on the flattened closure.
//TEST:SIMPLE(filecheck=CHECK_VER_OK): -target glsl -entry main -stage compute -capability spirv_1_5
// CHECK_VER_OK: result code = 0
// CHECK_VER_OK-NOT: E36121

// Each -capability option is classified independently: the extension is diagnosed while the version
// option stays exempt (the -NOT guards against a spurious E36121 on spirv_1_5).
//TEST:SIMPLE(filecheck=CHECK_MIXED): -target glsl -entry main -stage compute -capability spirv_1_5 -capability SPV_KHR_ray_tracing
// CHECK_MIXED: error[E36121]
// CHECK_MIXED: SPV_KHR_ray_tracing
// CHECK_MIXED-NOT: spirv_1_5

// sm_6_0 is compatible with a GLSL target via its _GLSL_450 disjunct (capdef sm_6_0_version), so it
// must not be flagged.
//TEST:SIMPLE(filecheck=CHECK_CROSS_OK): -target glsl -entry main -stage compute -capability sm_6_0
// CHECK_CROSS_OK: result code = 0
// CHECK_CROSS_OK-NOT: E36121

[numthreads(1, 1, 1)]
void main() {}
