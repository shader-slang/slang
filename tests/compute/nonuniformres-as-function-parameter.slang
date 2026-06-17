//TEST:SIMPLE(filecheck=CHECK_SPV):-target spirv -entry main -stage compute -O0
//TEST:SIMPLE(filecheck=CHECK_GLSL_SPV):-target spirv -entry main -stage compute -emit-spirv-via-glsl -O0
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

    // NonUniform propagation is pinned per *callee specialization* rather than
    // at the call site: Slang specializes `func` per argument type/uniformity,
    // so the access chain into `globalBuffer` lives inside the specialized
    // callee, not (necessarily) inlined at the call site. This keeps the test
    // independent of whether a downstream optimizer happens to inline calls.
    //   - case 1 (uint `bufferIdx`):       calls `func`,   callee decorated
    //   - case 2 (int `bufferIdx2`):       calls `func_0`, callee decorated
    //   - case 3 (uniform `bufferIdx3`):   calls `func_1`, callee NOT decorated
    //   - case 4 (cast of a NonUniformResourceIndex back to uint, so it
    //     collapses to the same specialization as case 1): calls `func` again
    // CHECK_SPV: OpFunctionCall %MyStruct %func %bufferIdx{{[[:space:]]*$}}

    uint bufferIdx = pixelIndex.x;
    uint nonUniformIdx = NonUniformResourceIndex(bufferIdx);
    RWStructuredBuffer<uint> buffer = globalBuffer[nonUniformIdx];

    // CHECK_GLSL_SPV: OpFunctionCall %MyStruct_0 %func_0_u1_

    // CHECK_GLSL: func_0({{.*}}nonuniformEXT({{.*}}))
    // CHECK_HLSL: func_0(globalBuffer_0[NonUniformResourceIndex({{.*}})])
    // CHECK_CUDA: func_{{[0-9]+}}(globalParams_{{[0-9]+}}->globalBuffer_{{[0-9]+}}[{{.*}}])
    MyStruct myStruct = func(buffer);

    int bufferIdx2 = pixelIndex.y;

    // Test case 2: Make sure we cover the case for the different data type of the index.
    // In this case, slang will specialize the function to 'MyStruct func(int)'.
    // CHECK_SPV: OpFunctionCall %MyStruct %func_0 %bufferIdx2

    // CHECK_GLSL_SPV: OpFunctionCall %MyStruct_0 %func_1_i1_
    RWStructuredBuffer<uint> buffer2 = globalBuffer[NonUniformResourceIndex(bufferIdx2)];

    // CHECK_GLSL: func_1({{.*}}nonuniformEXT({{.*}}))
    // CHECK_HLSL: func_0(globalBuffer_0[NonUniformResourceIndex({{.*}})])
    // CHECK_CUDA: func_{{[0-9]+}}(globalParams_{{[0-9]+}}->globalBuffer_{{[0-9]+}}[{{.*}}])
    MyStruct myStruct2 = func(buffer2);

    // Test case 3: a uniform index must NOT propagate NonUniform into the callee.
    // CHECK_SPV: OpFunctionCall %MyStruct %func_1 %bufferIdx3
    int bufferIdx3 = pixelIndex.z;

    RWStructuredBuffer<uint> buffer3 = globalBuffer[bufferIdx3];
    // CHECK_GLSL_SPV: OpFunctionCall %MyStruct_0 %func_2_i1_
    // CHECK_CUDA: func_{{[0-9]+}}(globalParams_{{[0-9]+}}->globalBuffer_{{[0-9]+}}[{{.*}}])
    MyStruct myStruct3 = func(buffer3);

    // Test case 4: an intCast/uintCast of a NonUniformResourceIndex is still a
    // NonUniformResourceIndex. Since after the nested cast, the index data type
    // is 'uint' now, make sure it calls the same specialization as test case 1.
    // CHECK_SPV: OpFunctionCall %MyStruct %func %{{[0-9]+}}
    // CHECK_GLSL_SPV: OpFunctionCall %MyStruct_0 %func_0_u1_
    // CHECK_GLSL: func_0({{.*}}nonuniformEXT({{.*}}))
    // CHECK_CUDA: func_{{[0-9]+}}(globalParams_{{[0-9]+}}->globalBuffer_{{[0-9]+}}[{{.*}}])
    RWStructuredBuffer<uint> buffer4 = globalBuffer[(uint)((int)NonUniformResourceIndex(bufferIdx))];
    MyStruct myStruct4 = func(buffer4);

    // Now check the decoration inside each specialized callee, in their
    // definition order. `func`/`func_0_u1_` and `func_0`/`func_1_i1_` are the
    // nonuniform-index specializations (decorated); `func_1`/`func_2_i1_` is
    // the uniform-index specialization (NOT decorated -- its access chain
    // line ends right after the index, with no "; NonUniform" annotation).
    // CHECK_SPV: %func = OpFunction
    // CHECK_SPV: OpAccessChain {{.*}} %globalBuffer {{.*}} ; NonUniform
    // CHECK_SPV: %func_0 = OpFunction
    // CHECK_SPV: OpAccessChain {{.*}} %globalBuffer {{.*}} ; NonUniform
    // CHECK_SPV: %func_1 = OpFunction
    // CHECK_SPV: OpAccessChain %_ptr_StorageBuffer_RWStructuredBuffer %globalBuffer %{{[A-Za-z0-9_]+}}{{[[:space:]]*$}}

    // CHECK_GLSL_SPV: %func_0_u1_ = OpFunction
    // CHECK_GLSL_SPV: OpCopyObject %uint %{{.*}} ; NonUniform
    // CHECK_GLSL_SPV: OpAccessChain {{.*}} %globalBuffer_0 {{.*}} ; NonUniform
    // CHECK_GLSL_SPV: %func_1_i1_ = OpFunction
    // CHECK_GLSL_SPV: OpCopyObject %int %{{.*}} ; NonUniform
    // CHECK_GLSL_SPV: OpAccessChain {{.*}} %globalBuffer_0 {{.*}} ; NonUniform
    // CHECK_GLSL_SPV: %func_2_i1_ = OpFunction
    // CHECK_GLSL_SPV: OpAccessChain %_ptr_Uniform_uint %globalBuffer_0 %{{[A-Za-z0-9_]+}} %int_0 %int_0{{[[:space:]]*$}}

    outputBuffer[0] = uint3(myStruct.a, myStruct.b, myStruct.c);
    outputBuffer[1] = uint3(myStruct2.a, myStruct2.b, myStruct2.c);
    outputBuffer[2] = uint3(myStruct3.a, myStruct3.b, myStruct3.c);
    outputBuffer[3] = uint3(myStruct4.a, myStruct4.b, myStruct4.c);
}

