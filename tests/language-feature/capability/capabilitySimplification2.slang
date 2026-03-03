//TEST:SIMPLE(filecheck=SPIRV): -target spirv -emit-spirv-directly -entry computeMain -stage compute -profile spirv_1_3
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute -profile sm_5_0
//TEST:SIMPLE(filecheck=HLSL): -target hlsl  -entry computeMain -stage compute -profile sm_5_0
//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS): -target spirv -emit-spirv-directly -entry computeMain -stage compute  -profile spirv_1_3 -ignore-capabilities


// CHECK_IGNORE_CAPS-NOT: E41012

// SPIRV: warning[E41012]
// SPIRV-NOT: spirv_1_2
// SPIRV: spvGroupNonUniformBallot

// GLSL: warning[E41012]
// GLSL-NOT: GLSL_400
// GLSL-NOT: GLSL_430
// GLSL: GL_KHR_shader_subgroup_basic

// HLSL: warning[E41012]
// HLSL-NOT: sm_5_1
// HLSL: sm_6_0

[numthreads(1,1,1)]
void computeMain()
{
    if (WaveIsFirstLane())
        return;
}
