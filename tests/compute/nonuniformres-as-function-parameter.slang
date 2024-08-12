//TEST:SIMPLE(filecheck=CHECK_SPV):-target spirv -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_GLSL_SPV):-target spirv -entry main -stage compute -emit-spirv-via-glsl
//TEST:SIMPLE(filecheck=CHECK_GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage compute
RWStructuredBuffer<uint> globalBuffer[] : register(t0, space0);
RWStructuredBuffer<uint> globalBuffer1[] : register(t1, space0);
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

    // CHECK_GLSL: globalBuffer_0[nonuniformEXT({{.*}})]
    // CHECK_GLSL: globalBuffer_0[nonuniformEXT({{.*}})]

    // For the last test case that the callee passes globalBuffer1,
    // we should not see nonuniformEXT here.

    // CHECK_GLSL: globalBuffer1_0[_{{.*}})]
    // CHECK_GLSL: globalBuffer1_0[_{{.*}})]
    a.a = buffer[0];
    a.b = a.a + 1;
    a.c = a.a + a.b + 1;

    return a;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint2 pixelIndex : SV_DispatchThreadID)
{

    // CHECK_SPV: OpDecorate %[[VAR1:[a-zA-Z0-9_]+]] NonUniform
    // CHECK_SPV: OpDecorate %[[VAR2:[a-zA-Z0-9_]+]] NonUniform
    // CHECK_SPV-NOT: OpDecorate %[[VAR3:[a-zA-Z0-9_]+]] NonUniform


    // CHECK_GLSL_SPV: OpDecorate %[[VAR1:[a-zA-Z0-9_]+]] NonUniform
    // CHECK_GLSL_SPV: OpDecorate %[[VAR2:[a-zA-Z0-9_]+]] NonUniform
    // CHECK_GLSL_SPV: OpDecorate %[[VAR3:[a-zA-Z0-9_]+]] NonUniform
    // CHECK_GLSL_SPV: OpDecorate %[[VAR4:[a-zA-Z0-9_]+]] NonUniform
    // CHECK_GLSL_SPV: OpDecorate %[[VAR5:[a-zA-Z0-9_]+]] NonUniform
    // CHECK_GLSL_SPV: OpDecorate %[[VAR6:[a-zA-Z0-9_]+]] NonUniform
    // CHECK_GLSL_SPV: OpDecorate %[[VAR7:[a-zA-Z0-9_]+]] NonUniform
    // CHECK_GLSL_SPV: OpDecorate %[[VAR8:[a-zA-Z0-9_]+]] NonUniform
    // CHECK_GLSL_SPV: OpDecorate %[[VAR9:[a-zA-Z0-9_]+]] NonUniform
    // CHECK_GLSL_SPV: OpDecorate %[[VAR10:[a-zA-Z0-9_]+]] NonUniform
    // CHECK_GLSL_SPV: OpDecorate %[[VAR11:[a-zA-Z0-9_]+]] NonUniform
    // CHECK_GLSL_SPV: OpDecorate %[[VAR12:[a-zA-Z0-9_]+]] NonUniform


    // CHECK_GLSL_SPV: OpCopyObject %int %{{.*}}
    uint bufferIdx = pixelIndex.x;
    uint nonUniformIdx = NonUniformResourceIndex(bufferIdx);
    RWStructuredBuffer<uint> buffer = globalBuffer[nonUniformIdx];

    // CHECK_SPV: %[[VAR1]] = OpAccessChain %_ptr_StorageBuffer_RWStructuredBuffer{{.*}} %{{.*}} %bufferIdx

    // CHECK_GLSL_SPV: %[[VAR1:[a-zA-Z0-9_]+]] = OpCopyObject %int %{{.*}}
    // CHECK_GLSL_SPV: %[[VAR2:[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_uint %globalBuffer_0 %[[VAR1]] %int_0 %int_0
    // CHECK_GLSL_SPV: %[[VAR3:[a-zA-Z0-9_]+]] = OpLoad %uint %[[VAR2]]
    //
    // CHECK_GLSL_SPV: %[[VAR4:[a-zA-Z0-9_]+]] = OpCopyObject %int %{{.*}}
    // CHECK_GLSL_SPV: %[[VAR5:[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_uint %globalBuffer_0 %[[VAR4]] %int_0 %int_0
    // CHECK_GLSL_SPV: %[[VAR6:[a-zA-Z0-9_]+]] = OpLoad %uint %[[VAR5]]

    // CHECK_GLSL: func_0({{.*}}nonuniformEXT({{.*}}))
    // CHECK_HLSL: func_0(globalBuffer_0[NonUniformResourceIndex({{.*}})])
    MyStruct myStruct = func(buffer);

    int bufferIdx2 = pixelIndex.y;

    // CHECK_SPV: %[[VAR2]] = OpAccessChain %_ptr_StorageBuffer_RWStructuredBuffer{{.*}} %{{.*}} %bufferIdx2


    // CHECK_GLSL-SPV: %[[VAR7:[a-zA-Z0-9_]+]] = OpCopyObject %int %{{.*}}
    // CHECK_GLSL-SPV: %[[VAR8:[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_uint %globalBuffer_0 %[[VAR7]] %int_0 %int_0
    // CHECK_GLSL-SPV: %[[VAR9:[a-zA-Z0-9_]+]] = OpLoad %uint %[[VAR8]]

    // CHECK_GLSL-SPV: %[[VAR10:[a-zA-Z0-9_]+]] = OpCopyObject %int %{{.*}}
    // CHECK_GLSL-SPV: %[[VAR11:[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_uint %globalBuffer_0 %[[VAR10]] %int_0 %int_0
    // CHECK_GLSL-SPV: %[[VAR12:[a-zA-Z0-9_]+]] = OpLoad %uint %[[VAR11]]
    RWStructuredBuffer<uint> buffer2 = globalBuffer[NonUniformResourceIndex(bufferIdx2)];

    // CHECK_GLSL: func_1({{.*}}nonuniformEXT({{.*}}))
    // CHECK_HLSL: func_0(globalBuffer_0[NonUniformResourceIndex({{.*}})])
    MyStruct myStruct2 = func(buffer2);


    int bufferIdx3 = pixelIndex.y;
    RWStructuredBuffer<uint> buffer3 = globalBuffer1[bufferIdx3];

    // Test to make sure this command is not decorated with NonUniform:
    // see the check: "CHECK_SPV-NOT: OpDecorate %[[VAR3:[a-zA-Z0-9_]+]] NonUniform" above.
    // CHECK_SPV: %[[VAR3:[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_StorageBuffer_RWStructuredBuffer{{.*}} %{{.*}} %bufferIdx2
    MyStruct myStruct3 = func(buffer3);

    outputBuffer[0] = uint3(myStruct.a, myStruct.b, myStruct.c);
    outputBuffer[1] = uint3(myStruct2.a, myStruct2.b, myStruct2.c);
    outputBuffer[2] = uint3(myStruct3.a, myStruct3.b, myStruct3.c);
}

