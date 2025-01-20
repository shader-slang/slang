//DISABLE_TEST:CROSS_COMPILE(filecheck=CHECK): -profile glsl_450+spirv_1_4 -stage compute -entry computeMain -target spirv-assembly -emit-spirv-via-glsl

RWStructuredBuffer<float> outputBuffer;

StructuredBuffer<float> buf;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // CHECK: %[[VAR1:[a-zA-Z0-9_]+]] = OpTypeCooperativeVectorNV %int %uint_16
    // CHECK: OpConstantCompositeReplicateEXT %9 %int_1
    coopvecNV<int, 16> v = coopvecNV<int, 16>(1);

    // CHECK: %[[VAR2:[a-zA-Z0-9_]+]] = OpTypeCooperativeVectorNV %int %uint_8
    coopvecNV<int, 8> r;
    int offset = 0;
    int layout = gl_CooperativeVectorMatrixLayoutRowMajorNV;
    bool transpose = false;
    int matrixStride = 4;

    // CHECK: OpCooperativeVectorMatrixMulNV %[[VAR2]] %{{.*}} %int_1 %{{.*}} %uint_0 %int_1 %uint_8 %uint_16 %int_0 %false %uint_4 MatrixBSignedComponentsKHR|MatrixResultSignedComponentsKHR
    coopVecMatMulNV(
        r,
        v,
        gl_ComponentTypeFloat32NV,
        buf,
        offset,
        gl_ComponentTypeFloat32NV,
        8,
        16,
        layout,
        transpose,
        matrixStride);
    outputBuffer[dispatchThreadID.x] = r[0];
}
