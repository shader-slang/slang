// Regression for #11141: compiling a `WaveGetLaneIndex` shader at
// `-profile sm_6_0 -target spirv` used to emit
// `warning[E41012]: profile implicitly upgraded ... 'spvGroupNonUniform'`
// because the `sm_6_0` capability alias bundled five
// `GL_KHR_shader_subgroup_*` extensions for the SPIR-V branch but not
// `GL_KHR_shader_subgroup_basic`.  HLSL SM 6.0 includes the basic wave
// intrinsics (WaveGetLaneIndex/WaveGetLaneCount/WaveIsFirstLane); the
// SPIR-V/GLSL mapping for `sm_6_0` should match.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -profile sm_6_0 -stage compute -entry main

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = WaveGetLaneIndex();
}

// Compilation must succeed without an implicit-profile-upgrade warning
// pointing at spvGroupNonUniform.
// CHECK-NOT: warning[E41012]
// CHECK-NOT: spvGroupNonUniform
// CHECK: OpEntryPoint
