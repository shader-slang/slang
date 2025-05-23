//TEST:SIMPLE(filecheck=CHECK_GLSL): -stage compute -entry computeMain -allow-glsl -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV): -stage compute -entry computeMain -allow-glsl -target spirv -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_HLSL): -stage compute -entry computeMain -target hlsl -allow-glsl
//TEST:SIMPLE(filecheck=CHECK_CUDA): -stage compute -entry computeMain -target cuda -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -emit-spirv-directly

#version 450

//TEST_INPUT:ubuffer(data=[0 1 2 3  4 5 6 7  8 9 10 11  12 13 14 15], stride=4):name=inputBuffer
layout(binding = 0) buffer MyBlockNameIn
{
    uvec4 a[4];
} inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0  0 0 0 0  0 0 0 0], stride=4):out,name=outputBuffer
layout(binding = 1) buffer MyBlockName
{
    uvec4 a;
    uvec4 b;
    uvec3 c;
} outputBuffer;

layout(local_size_x = 1) in;
void computeMain()
{
    {
        f16vec4 m0 = f16vec4(inputBuffer.a[0]);
        f16vec4 m1 = f16vec4(inputBuffer.a[1]);
        f16vec4 m2 = f16vec4(inputBuffer.a[2]);
        f16vec4 m3 = f16vec4(inputBuffer.a[3]);
        f16mat4 m = f16mat4(m0, m1, m2, m3);
        f16vec4 res = m * f16vec4(1, 1, 1, 1);
        outputBuffer.a = uvec4(res);
        // BUF: 18
        // BUF-NEXT: 1C
        // BUF-NEXT: 20
        // BUF-NEXT: 24
    }

    {
        f16vec4 m0 = f16vec4(inputBuffer.a[0]);
        f16vec4 m1 = f16vec4(inputBuffer.a[1]);
        f16vec4 m2 = f16vec4(inputBuffer.a[2]);
        f16mat3x4 m = f16mat3x4(m0, m1, m2);
        f16vec4 res = m * f16vec3(1, 1, 1);
        outputBuffer.b = uvec4(res);
        // BUF-NEXT: C
        // BUF-NEXT: F
        // BUF-NEXT: 12
        // BUF-NEXT: 15
    }

    {
        f16vec3 m0 = f16vec3(inputBuffer.a[0].xyz);
        f16vec3 m1 = f16vec3(inputBuffer.a[1].xyz);
        f16mat2x3 m = f16mat2x3(m0, m1);
        f16vec3 res = m * f16vec2(1, 1);
        outputBuffer.c = uvec3(res);
        // BUF-NEXT: 4
        // BUF-NEXT: 6
        // BUF-NEXT: 8
    }



    // CHECK_GLSL: f16mat4x4
    // CHECK_GLSL: f16vec4
    // CHECK_GLSL: f16mat3x4
    // CHECK_GLSL: f16mat2x3
    // CHECK_GLSL: f16vec3
    // CHECK_GLSL: f16vec2

    // CHECK_SPV: OpTypeFloat 16

    // CHECK_HLSL: vector<half,4>
    // CHECK_HLSL: matrix<half,int(4),int(4)>
    // CHECK_HLSL: matrix<half,int(3),int(4)>
    // CHECK_HLSL: matrix<half,int(2),int(3)>

    // CHECK_CUDA: __half4
    // CHECK_CUDA: makeMatrix<__half, 4, 4>
    // CHECK_CUDA: makeMatrix<__half, 3, 4>
    // CHECK_CUDA: makeMatrix<__half, 2, 3>
}
