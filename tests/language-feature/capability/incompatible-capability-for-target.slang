//TEST:SIMPLE(filecheck=CHECK_ERR): -target hlsl -entry main -stage compute -capability spirv_1_5
//TEST:SIMPLE(filecheck=CHECK_OK): -target spirv -entry main -stage compute -capability spirv_1_5
//TEST:SIMPLE(filecheck=CHECK_OK): -target spirv -entry main -stage compute -capability spirv_1_5 -ignore-capabilities

// Requesting a SPIRV capability with an HLSL target should be an error.
// CHECK_ERR: error[E36121]
// CHECK_OK-NOT: E36121

[numthreads(1, 1, 1)]
void main() {}
