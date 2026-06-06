//TEST:SIMPLE(filecheck=CHECK_SPV):-target spirv -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_GLSL_SPV):-target spirv -entry main -stage compute -emit-spirv-via-glsl
//TEST:SIMPLE(filecheck=CHECK_GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_CUDA):-target cuda -entry main -stage compute
RWStructuredBuffer<uint> globalBuffer[] : register(u0, space1);
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

    // For the last test case 3 that the callee passes globalBuffer[bufferIdx3] to the function,
    // we should not see nonuniformEXT here.

    // CHECK_GLSL: globalBuffer_0[_{{.*}})]
    // CHECK_GLSL: globalBuffer_0[_{{.*}})]
    a.a = buffer[0];
    a.b = a.a + 1;
    a.c = a.a + a.b + 1;

    return a;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 pixelIndex : SV_DispatchThreadID)
{

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
    // CHECK_GLSL_SPV: OpDecorate %[[VAR13:[a-zA-Z0-9_]+]] NonUniform
    // CHECK_GLSL_SPV: OpDecorate %[[VAR14:[a-zA-Z0-9_]+]] NonUniform
    // CHECK_GLSL_SPV: OpDecorate %[[VAR15:[a-zA-Z0-9_]+]] NonUniform
    // CHECK_GLSL_SPV: OpDecorate %[[VAR16:[a-zA-Z0-9_]+]] NonUniform
    // CHECK_GLSL_SPV: OpDecorate %[[VAR17:[a-zA-Z0-9_]+]] NonUniform
    // CHECK_GLSL_SPV: OpDecorate %[[VAR18:[a-zA-Z0-9_]+]] NonUniform

    // Direct-SPIRV decoration coverage for all four cases at once.
    //
    // slang-test appends "; NonUniform" inline to decorated instructions. Three
    // of the four globalBuffer descriptor-array accesses use a non-uniform index
    // and so must have NonUniform-decorated access chains:
    //   - case 1: uint index `bufferIdx`
    //   - case 2: int index `bufferIdx2`
    //   - case 4: (uint)((int)NonUniformResourceIndex(bufferIdx)) -- the cast
    //             round-trip collapses back to `bufferIdx` on the direct backend,
    //             so it produces the same access-chain shape as case 1
    // and case 3 (uniform `bufferIdx3`) must NOT be decorated. Asserting exactly
    // three decorated descriptor access chains therefore covers every case in
    // one order-independent check: a regression that dropped NonUniform across
    // the Int/Uint casts (case 4) would drop the count to two, and a spurious
    // decoration of the uniform case 3 would raise it to four. A count is used
    // instead of per-case `%bufferIdx{n}` bindings because cases 1 and 4 share
    // the same index after the cast collapses, and because it does not depend on
    // FileCheck cursor position relative to the annotations section.
    // CHECK_SPV-COUNT-3: = OpAccessChain %_ptr_StorageBuffer_RWStructuredBuffer{{.*}} %globalBuffer{{.*}}; NonUniform
    // CHECK_SPV-NOT: = OpAccessChain %_ptr_StorageBuffer_RWStructuredBuffer{{.*}} %globalBuffer{{.*}}; NonUniform

    uint bufferIdx = pixelIndex.x;
    uint nonUniformIdx = NonUniformResourceIndex(bufferIdx);
    RWStructuredBuffer<uint> buffer = globalBuffer[nonUniformIdx];

    // CHECK_GLSL_SPV: %[[VAR1]] = OpCopyObject %uint %{{.*}}

    // CHECK_GLSL_SPV: %[[VAR4]] = OpCopyObject %uint %[[VAR1]]
    // CHECK_GLSL_SPV: %[[VAR5]] = OpAccessChain %_ptr_Uniform_uint %globalBuffer_0 %[[VAR4]] %int_0 %int_0
    // CHECK_GLSL_SPV: %[[VAR6]] = OpLoad %uint %[[VAR5]]

    // CHECK_GLSL_SPV: %[[VAR7]] = OpCopyObject %uint %[[VAR1]]
    // CHECK_GLSL_SPV: %[[VAR8]] = OpAccessChain %_ptr_Uniform_uint %globalBuffer_0 %[[VAR7]] %int_0 %int_0
    // CHECK_GLSL_SPV: %[[VAR9]] = OpLoad %uint %[[VAR8]]

    // CHECK_GLSL: func_0({{.*}}nonuniformEXT({{.*}}))
    // CHECK_HLSL: func_0(globalBuffer_0[NonUniformResourceIndex({{.*}})])
    // CHECK_CUDA: func_{{[0-9]+}}(globalParams_{{[0-9]+}}->globalBuffer_{{[0-9]+}}[{{.*}}])
    MyStruct myStruct = func(buffer);

    int bufferIdx2 = pixelIndex.y;

    // Test case 2: Make sure we cover the case for the different data type of the index.
    // In this case, slang will specialize the function to 'MyStruct func(int)'
    // The access chain for bufferIdx2 should be decorated NonUniform (covered by
    // the decorated-descriptor-chain count check above).

    // CHECK_GLSL_SPV: %[[VAR2]] = OpCopyObject %int %{{.*}}

    // CHECK_GLSL-SPV: %[[VAR10]] = OpCopyObject %int %[[VAR2]]
    // CHECK_GLSL-SPV: %[[VAR11]] = OpAccessChain %_ptr_Uniform_uint %globalBuffer_0 %[[VAR10]] %int_0 %int_0
    // CHECK_GLSL-SPV: %[[VAR12]] = OpLoad %uint %[[VAR11]]

    // CHECK_GLSL-SPV: %[[VAR13]] = OpCopyObject %int %[[VAR2]]
    // CHECK_GLSL-SPV: %[[VAR14]] = OpAccessChain %_ptr_Uniform_uint %globalBuffer_0 %[[VAR13]] %int_0 %int_0
    // CHECK_GLSL-SPV: %[[VAR15]] = OpLoad %uint %[[VAR14]]
    RWStructuredBuffer<uint> buffer2 = globalBuffer[NonUniformResourceIndex(bufferIdx2)];

    // CHECK_GLSL: func_1({{.*}}nonuniformEXT({{.*}}))
    // CHECK_HLSL: func_0(globalBuffer_0[NonUniformResourceIndex({{.*}})])
    // CHECK_CUDA: func_{{[0-9]+}}(globalParams_{{[0-9]+}}->globalBuffer_{{[0-9]+}}[{{.*}}])
    MyStruct myStruct2 = func(buffer2);

    // Test case 3: a uniform index must NOT propagate NonUniform into the callee,
    // so the access chain for the uniform bufferIdx3 must NOT be decorated
    // NonUniform. This is covered by the decorated-descriptor-chain count check
    // above: a spurious decoration here would make four decorated chains.
    int bufferIdx3 = pixelIndex.z;

    RWStructuredBuffer<uint> buffer3 = globalBuffer[bufferIdx3];
    // CHECK_CUDA: func_{{[0-9]+}}(globalParams_{{[0-9]+}}->globalBuffer_{{[0-9]+}}[{{.*}}])
    MyStruct myStruct3 = func(buffer3);

    // CHECK_GLSL-SPV: %[[VAR19:[a-zA-Z0-9_]+]]  = OpBitcast %int %[[VAR3]]
    // CHECK_GLSL-SPV: %[[VAR16]] = OpCopyObject %int %[[VAR19]]
    // CHECK_GLSL-SPV: %[[VAR17]] = OpAccessChain %_ptr_Uniform_uint %globalBuffer_0 %[[VAR16]] %int_0 %int_0
    // CHECK_GLSL-SPV: %[[VAR18]] = OpLoad %uint %[[VAR17]]
    //
    // Test case 4: an intCast/uintCast of a NonUniformResourceIndex is still a
    // NonUniformResourceIndex.
    // Since after the nested cast, the index data type is 'uint' now, make sure it calls the same function as the test case 1.
    // CHECK_GLSL: func_0({{.*}}nonuniformEXT({{.*}}))
    // CHECK_CUDA: func_{{[0-9]+}}(globalParams_{{[0-9]+}}->globalBuffer_{{[0-9]+}}[{{.*}}])
    RWStructuredBuffer<uint> buffer4 = globalBuffer[(uint)((int)NonUniformResourceIndex(bufferIdx))];
    MyStruct myStruct4 = func(buffer4);

    outputBuffer[0] = uint3(myStruct.a, myStruct.b, myStruct.c);
    outputBuffer[1] = uint3(myStruct2.a, myStruct2.b, myStruct2.c);
    outputBuffer[2] = uint3(myStruct3.a, myStruct3.b, myStruct3.c);
    outputBuffer[3] = uint3(myStruct4.a, myStruct4.b, myStruct4.c);
}

