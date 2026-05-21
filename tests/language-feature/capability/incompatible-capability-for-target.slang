// Requesting a SPIRV capability on an HLSL target should error.
//TEST:SIMPLE(filecheck=CHECK_ERR): -target hlsl -entry main -stage compute -capability spirv_1_5
// CHECK_ERR: error[E36121]
// CHECK_ERR: spirv_1_5

// The same SPIRV capability on a SPIRV target is fine.
//TEST:SIMPLE(filecheck=CHECK_OK): -target spirv -entry main -stage compute -capability spirv_1_5
// CHECK_OK: OpEntryPoint
// CHECK_OK-NOT: E36121

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

// Note: cross-target version aliases like sm_6_0 intentionally include SPIRV paths
// (sm_6_0 maps to spirv_1_3 for SPIRV targets), so they do NOT trigger E36121.

[numthreads(1, 1, 1)]
void main() {}
