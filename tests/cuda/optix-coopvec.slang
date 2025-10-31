//TEST:SIMPLE(filecheck=CHECK): -target cuda -capability optix_coopvec

// Note - Validated locally.Enable with latest optix and cuda SDK
// TODO: GitHub issue #8556 needs to be resolved to enable the test when OptiX 9.0+ is released
//DISABLE_TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -capability optix_coopvec

// CHECK-PTX: _optix_vector_load_16xi32
// CHECK-PTX: _optix_vector_op1_16xi32
// CHECK-PTX: _optix_vector_op2_16xi32
// CHECK-PTX: _optix_vector_op3_16xi32
// CHECK-PTX: _optix_matvecmul_16xi32
// CHECK-PTX: _optix_outer_product_accumulate_16xi32
// CHECK-PTX: _optix_reduce_sum_accumulate_16xi32
// CHECK-PTX: add.f32
// CHECK: (optixCoopVecLoad<OptixCoopVec<float, 4>>((CUdeviceptr)(&((globalParams_{{[0-9]+}}->input{{[0-9]+}}_{{[0-9]+}})))))
// CHECK: (optixCoopVecTanh((_S{{[0-9]+}})))
// CHECK: (optixCoopVecAdd((_S{{[0-9]+}}), (_S{{[0-9]+}})))
// CHECK: optixCoopVecCvt<OptixCoopVec<float, int(4)>>(*((&resultCopy_{{[0-9]+}})))
// CHECK: (optixCoopVecFFMA((_S{{[0-9]+}}), (_S{{[0-9]+}}), (_S{{[0-9]+}})))
// CHECK: (slangOptixCoopVecMatMul<{{.*}}>(({{.*}}), (CUdeviceptr)(&((globalParams_{{[0-9]+}}->{{.*}}))), {{.*}})
// CHECK: (optixCoopVecMax((_S{{[0-9]+}}), (_S{{[0-9]+}})))
// CHECK: (optixCoopVecMin((_S{{[0-9]+}}), (_S{{[0-9]+}})))
// CHECK: (optixCoopVecMul((_S{{[0-9]+}}), (_S{{[0-9]+}})))
// CHECK: optixCoopVecOuterProductAccumulate((_S{{[0-9]+}}), (_S{{[0-9]+}}), (CUdeviceptr)(&(globalParams_{{[0-9]+}}->outputMat_{{[0-9]+}})), (int(0)), (32U))
// CHECK: optixCoopVecReduceSumAccumulate((_S{{[0-9]+}}), (CUdeviceptr)(&(globalParams_{{[0-9]+}}->outputMat{{[0-9]+}}_{{[0-9]+}})), (int(0)))
// CHECK: (optixCoopVecStep((_S{{[0-9]+}}), (_S{{[0-9]+}})))
// CHECK: (optixCoopVecSub((_S{{[0-9]+}}), (_S{{[0-9]+}})))
// CHECK: (optixCoopVecLog2((_S{{[0-9]+}})))
// CHECK: (optixCoopVecExp2((_S{{[0-9]+}})))


//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4),name=input1
ByteAddressBuffer input1;

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4),name=input2
ByteAddressBuffer input2;

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4),name=input3
ByteAddressBuffer input3;

//TEST_INPUT: set inputBuffer = ubuffer(data=[1 2 3 4 5 6 7 8 9 10 11 12], stride=4);
uniform int32_t* inputBuffer;

//TEST_INPUT:ubuffer(data=[67305985 134678021 202050057 269422093], stride=4),name=matrix
//[1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16]
ByteAddressBuffer matrix;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4),name=outputMat
RWByteAddressBuffer outputMat;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4),name=outputMat2
RWByteAddressBuffer outputMat2;

//TEST_INPUT:ubuffer(data=[5 6 7 8], stride=4),name=bias
ByteAddressBuffer bias;

//TEST_INPUT:ubuffer(data=[67305985 134678021 202050057 269422093], stride=4),name=matrixSimple
//[1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16] - simple 4x4 matrix for coopVecMatMul test
ByteAddressBuffer matrixSimple;

//TEST_INPUT:ubuffer(data=[16909060 33752068 50595076 67438084], stride=4),name=biasSimple
//[4 3 2 1 4 3 2 1 4 3 2 1 4 3 2 1] - simple bias vector for coopVecMatMul with bias test
ByteAddressBuffer biasSimple;

//TEST_INPUT:ubuffer(data=[67305985 134678021 202050057 269422093], stride=4),name=matrixStructured
//[1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16] - StructuredBuffer matrix test
StructuredBuffer<float> matrixStructured;

//TEST_INPUT:ubuffer(data=[16909060 33752068 50595076 67438084], stride=4),name=biasStructured
//[4 3 2 1 4 3 2 1 4 3 2 1 4 3 2 1] - StructuredBuffer bias test
StructuredBuffer<float> biasStructured;



struct RayPayload
{
    float4 color;
    float2x4 lssData;
    bool isSphere;
    bool isLss;
};


[numthreads(1, 1, 1)]
[shader("closesthit")]
void closestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    CoopVec<float, 4> vec1 = coopVecLoad<4, float>(input1);
    CoopVec<float, 4> vec2 = coopVecLoad<4, float>(input2);
    CoopVec<float, 4> vec3 = coopVecLoad<4, float>(input3);

    CoopVec<float, 4> resultTan = tanh(vec1);

    let resultAdd = vec1 + vec2;

    CoopVec<float, 4> resultCopy = coopVecLoad<4, float>(input1);
    resultCopy.copyFrom<float>(vec2);

    CoopVec<float, 4> resultFMA = fma(vec1, vec2, vec3);
    
    CoopVec<float, 4> vec = coopVecLoad<4, float>(input1);
    let resultMul = coopVecMatMulAdd<float, 4, 4>(
        vec,
        CoopVecComponentType::Float32,
        matrix,
        0,
        CoopVecComponentType::Float32,
        bias,
        0,
        CoopVecComponentType::SignedInt32,
        CoopVecMatrixLayout::RowMajor,
        false,
        4
    );

    // Test coopVecMatMul (without bias addition)
    CoopVec<float, 4> resultMatMul = coopVecMatMul<float, 4, 4>(
        vec,
        CoopVecComponentType::Float32,
        matrixSimple,
        0,
        CoopVecComponentType::Float32,
        CoopVecMatrixLayout::RowMajor,
        false,
        4
    );

    // Test coopVecMatMul with different matrix interpretation (shows enum genericity)
    CoopVec<float, 4> resultMatMulInt8 = coopVecMatMul<float, 4, 4>(
        vec,
        CoopVecComponentType::Float32,
        matrixSimple,
        0,
        CoopVecComponentType::SignedInt8,
        CoopVecMatrixLayout::RowMajor,
        false,
        4
    );

    // These should trigger the StructuredBuffer function variants
    CoopVec<float, 4> resultMatMulStructured = coopVecMatMul<float, 4, 4>(
        vec,
        CoopVecComponentType::Float32,
        matrixStructured,  // StructuredBuffer<float> instead of ByteAddressBuffer
        0,
        CoopVecComponentType::Float32,
        CoopVecMatrixLayout::RowMajor,
        false,
        4
    );

    // Test case for StructuredBuffer with bias
    CoopVec<float, 4> resultMatMulAddStructured = coopVecMatMulAdd<float, 4, 4>(
        vec,
        CoopVecComponentType::Float32,
        matrixStructured,  // StructuredBuffer<float> matrix
        0,
        CoopVecComponentType::Float32,
        biasStructured,    // StructuredBuffer<float> bias
        0,
        CoopVecComponentType::Float32,
        CoopVecMatrixLayout::ColumnMajor,  // Different layout for variety
        false,
        4
    );

    CoopVec<float, 4> resultMax = max(vec1, vec2);
    CoopVec<float, 4> resultMin = min(vec1, vec2);
    
    CoopVec<float, 4> resultVecMul = vec1 * vec2;
    
    outputMat.Store<float>(0, float(1));
    coopVecOuterProductAccumulate(
        vec1,
        vec2,
        outputMat,
        0,
        32,
        CoopVecMatrixLayout::RowMajor,
        CoopVecComponentType::Float32,
    );

    outputMat2.Store(0, float(1));
    coopVecReduceSumAccumulate(
        vec1,
        outputMat2,
        0,
    );
    
    CoopVec<float, 4> resultStep = step(vec1, vec2);

    CoopVec<float, 4> resultSub = vec1 - vec2;
    
    CoopVec<float, 4> resultLog2 = log2(vec1);
    
    CoopVec<float, 4> resultExp2 = exp2(vec1);

    for(int i = 0; i < resultTan.getCount(); ++i)
    {
        outputBuffer[i] = resultTan[i]  +
                          resultAdd[i]  +
                          resultCopy[i] +
                          resultFMA[i]  +
                          resultMul[i]  +
                          resultMatMul[i] +
                          resultMatMulInt8[i] +
                          resultMatMulStructured[i] +
                          resultMatMulAddStructured[i] +
                          resultMax[i]  +
                          resultMin[i]  +
                          resultVecMul[i] +
                          outputMat.Load<float>(i)  +
                          outputMat2.Load<float>(i) +
                          resultStep[i] +
                          resultSub[i]  +
                          resultLog2[i] +
                          resultExp2[i];
    }
}

// Compute shader entry point for HLSL testing
[numthreads(1, 1, 1)]
void testCoopVecHLSL()
{
    CoopVec<float, 4> vec1 = coopVecLoad<4, float>(input1);
    CoopVec<float, 4> vec2 = coopVecLoad<4, float>(input2);

    CoopVec<float, 4> resultTan = tanh(vec1);
    CoopVec<float, 4> vec = coopVecLoad<4, float>(input1);

    // Call coopVecMatMul - this triggers static assertions in hlsl.meta.slang
    CoopVec<float, 4> resultMatMul = coopVecMatMul<float, 4, 4>(
        vec,
        CoopVecComponentType::Float32,
        matrixSimple,
        0,
        CoopVecComponentType::Float32,
        CoopVecMatrixLayout::RowMajor,
        false,
        4
    );

    // Call coopVecMatMulAdd - this also triggers static assertions
    let resultMul = coopVecMatMulAdd<float, 4, 4>(
        vec,
        CoopVecComponentType::Float32,
        matrix,
        0,
        CoopVecComponentType::Float32,
        bias,
        0,
        CoopVecComponentType::SignedInt32,
        CoopVecMatrixLayout::RowMajor,
        false,
        4
    );

    outputBuffer[0] = resultTan[0] + resultMatMul[0] + resultMul[0];
}
