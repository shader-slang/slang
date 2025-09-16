//TEST:SIMPLE(filecheck=CHECK): -target cuda -capability optix_coopvec

// Note - Validated locally.Enable with latest optix and cuda SDK
// TODO: GitHub issue #8556 needs to be resolved to enable the test when OptiX 9.0+ is released
//DISABLE_TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -capability optix_coopvec

// CHECK-PTX: _optix_vector_load_16xi32
// CHECK-PTX: _optix_vector_op1_16xi32
// CHECK-PTX: _optix_vector_op2_16xi32
// CHECK-PTX: _optix_vector_op3_16xi32
// CHECK-PTX: _optix_outer_product_accumulate_16xi32
// CHECK-PTX: _optix_reduce_sum_accumulate_16xi32
// CHECK-PTX: add.f32
// CHECK: OptixCoopVec<float, 4>{{.*}} = (optixCoopVecLoad<OptixCoopVec<float, 4> >((CUdeviceptr)(&((globalParams_{{[0-9]+}}->input{{[0-9]+}}_{{[0-9]+}})))))
// CHECK: OptixCoopVec<float, 4>{{.*}} = (optixCoopVecTanh((_S{{[0-9]+}})))
// CHECK: OptixCoopVec<float, 4>{{.*}} = (optixCoopVecAdd((_S{{[0-9]+}}), (_S{{[0-9]+}})))
// CHECK: optixCoopVecCvt<OptixCoopVec<float, int(4)>>(*((&resultCopy_{{[0-9]+}})))
// CHECK: OptixCoopVec<float, 4>{{.*}} = (optixCoopVecFFMA((_S{{[0-9]+}}), (_S{{[0-9]+}}), (_S{{[0-9]+}})))
// CHECK: OptixCoopVec<float, 4>{{.*}} = (optixCoopVecMax((_S{{[0-9]+}}), (_S{{[0-9]+}})))
// CHECK: OptixCoopVec<float, 4>{{.*}} = (optixCoopVecMin((_S{{[0-9]+}}), (_S{{[0-9]+}})))
// CHECK: OptixCoopVec<float, 4>{{.*}} = (optixCoopVecMul((_S{{[0-9]+}}), (_S{{[0-9]+}})))
// CHECK: optixCoopVecOuterProductAccumulate((_S{{[0-9]+}}), (_S{{[0-9]+}}), (CUdeviceptr)(&(globalParams_{{[0-9]+}}->outputMat_{{[0-9]+}})), (int(0)), (32U))
// CHECK: optixCoopVecReduceSumAccumulate((_S{{[0-9]+}}), (CUdeviceptr)(&(globalParams_{{[0-9]+}}->outputMat{{[0-9]+}}_{{[0-9]+}})), (int(0)))
// CHECK: OptixCoopVec<float, 4>{{.*}} = (optixCoopVecStep((_S{{[0-9]+}}), (_S{{[0-9]+}})))
// CHECK: OptixCoopVec<float, 4>{{.*}} = (optixCoopVecSub((_S{{[0-9]+}}), (_S{{[0-9]+}})))
// CHECK: OptixCoopVec<float, 4>{{.*}} = (optixCoopVecLog2((_S{{[0-9]+}})))
// CHECK: OptixCoopVec<float, 4>{{.*}} = (optixCoopVecExp2((_S{{[0-9]+}})))


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
