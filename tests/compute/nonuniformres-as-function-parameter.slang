//TEST:SIMPLE(filecheck=CHECK-SPV):-target spirv -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK-GLSL-SPV):-target spirv -entry main -stage compute -emit-spirv-via-glsl
//TEST:SIMPLE(filecheck=CHECK-GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK-HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<uint> globalBuffer[] : register(t0, space0);
RWStructuredBuffer<uint3> outputBuffer;

struct MyStruct
{
    uint a;
    uint b;
    uint c;
};


MyStruct func(RWStructuredBuffer<uint> buffer)
{
    MyStruct a;

    // CHECK-GLSL: globalBuffer_0[nonuniformEXT({{.*}})]
    // CHECK-GLSL: globalBuffer_0[nonuniformEXT({{.*}})]
    a.a = buffer[0];
    a.b = a.a + 1;
    a.c = a.a + a.b + 1;

    return a;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint2 pixelIndex : SV_DispatchThreadID)
{
    uint bufferIdx = pixelIndex.x;

    // CHECK-SPV: OpDecorate %[[VAR1:[a-zA-Z0-9_]+]] NonUniform
    // CHECK-SPV: OpDecorate %[[VAR2:[a-zA-Z0-9_]+]] NonUniform


    // CHECK-GLSL-SPV: OpDecorate %[[VAR1:[a-zA-Z0-9_]+]] NonUniform
    // CHECK-GLSL-SPV: OpDecorate %[[VAR2:[a-zA-Z0-9_]+]] NonUniform
    // CHECK-GLSL-SPV: OpDecorate %[[VAR3:[a-zA-Z0-9_]+]] NonUniform
    // CHECK-GLSL-SPV: OpDecorate %[[VAR4:[a-zA-Z0-9_]+]] NonUniform
    // CHECK-GLSL-SPV: OpDecorate %[[VAR5:[a-zA-Z0-9_]+]] NonUniform
    // CHECK-GLSL-SPV: OpDecorate %[[VAR6:[a-zA-Z0-9_]+]] NonUniform
    // CHECK-GLSL-SPV: OpDecorate %[[VAR7:[a-zA-Z0-9_]+]] NonUniform
    // CHECK-GLSL-SPV: OpDecorate %[[VAR8:[a-zA-Z0-9_]+]] NonUniform
    // CHECK-GLSL-SPV: OpDecorate %[[VAR9:[a-zA-Z0-9_]+]] NonUniform
    // CHECK-GLSL-SPV: OpDecorate %[[VAR10:[a-zA-Z0-9_]+]] NonUniform
    // CHECK-GLSL-SPV: OpDecorate %[[VAR11:[a-zA-Z0-9_]+]] NonUniform
    // CHECK-GLSL-SPV: OpDecorate %[[VAR12:[a-zA-Z0-9_]+]] NonUniform

    // CHECK-GLSL: layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

    uint nonUniformIdx = NonUniformResourceIndex(bufferIdx);
    RWStructuredBuffer<uint> buffer = globalBuffer[nonUniformIdx];

    // CHECK-SPV: %[[VAR3:[a-zA-Z0-9_]+]] = OpBitcast %int %bufferIdx
    // CHECK-SPV: %[[VAR1]] = OpAccessChain %_ptr_StorageBuffer_RWStructuredBuffer{{.*}} %{{.*}} %[[VAR3]]

    // CHECK-GLSL-SPV: %[[VAR1:[a-zA-Z0-9_]+]] = OpCopyObject %int %{{.*}}
    // CHECK-GLSL-SPV: %[[VAR2:[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_uint %globalBuffer_0 %[[VAR1]] %int_0 %int_0
    // CHECK-GLSL-SPV: %[[VAR3:[a-zA-Z0-9_]+]] = OpLoad %uint %[[VAR2]]
    //
    // CHECK-GLSL-SPV: %[[VAR4:[a-zA-Z0-9_]+]] = OpCopyObject %int %{{.*}}
    // CHECK-GLSL-SPV: %[[VAR5:[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_uint %globalBuffer_0 %[[VAR4]] %int_0 %int_0
    // CHECK-GLSL-SPV: %[[VAR6:[a-zA-Z0-9_]+]] = OpLoad %uint %[[VAR5]]

    // CHECK-GLSL: func_0({{.*}}nonuniformEXT({{.*}}))
    // CHECK-HLSL: func_0(globalBuffer_0[NonUniformResourceIndex({{.*}})])
    MyStruct myStruct = func(buffer);

    // CHECK-GLSL-SPV: OpCopyObject %int %{{.*}}
    int bufferIdx2 = pixelIndex.y;

    // CHECK-SPV: %[[VAR2]] = OpAccessChain %_ptr_StorageBuffer_RWStructuredBuffer{{.*}} %{{.*}} %bufferIdx2


    // CHECK-GLSL-SPV: %[[VAR7:[a-zA-Z0-9_]+]] = OpCopyObject %int %{{.*}}
    // CHECK-GLSL-SPV: %[[VAR8:[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_uint %globalBuffer_0 %[[VAR7]] %int_0 %int_0
    // CHECK-GLSL-SPV: %[[VAR9:[a-zA-Z0-9_]+]] = OpLoad %uint %[[VAR8]]

    // CHECK-GLSL-SPV: %[[VAR10:[a-zA-Z0-9_]+]] = OpCopyObject %int %{{.*}}
    // CHECK-GLSL-SPV: %[[VAR11:[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_uint %globalBuffer_0 %[[VAR10]] %int_0 %int_0
    // CHECK-GLSL-SPV: %[[VAR12:[a-zA-Z0-9_]+]] = OpLoad %uint %[[VAR11]]
    RWStructuredBuffer<uint> buffer2 = globalBuffer[NonUniformResourceIndex(bufferIdx2)];

    // CHECK-GLSL: func_0({{.*}}nonuniformEXT({{.*}}))
    // CHECK-HLSL: func_0(globalBuffer_0[NonUniformResourceIndex({{.*}})])
    MyStruct myStruct2 = func(buffer2);

    outputBuffer[0] = uint3(myStruct.a, myStruct.b, myStruct.c);
    outputBuffer[1] = uint3(myStruct2.a, myStruct2.b, myStruct2.c);
}

