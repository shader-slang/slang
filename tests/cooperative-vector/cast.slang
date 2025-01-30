//DISABLE_TEST:CROSS_COMPILE: -profile glsl_450+spirv_1_4 -stage compute -entry computeMain -target spirv-assembly

RWStructuredBuffer<float> outputBuffer;

StructuredBuffer<float> buf;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    coopvecNV<int, 8> r_int;
    coopvecNV<float, 8> r = coopvecNV<float, 8>(r_int);
    coopvecNV<int, 16> v = coopvecNV<int, 16>(1);
    int offset = 0;
    int layout = gl_CooperativeVectorMatrixLayoutRowMajorNV;
    bool transpose = false;
    int matrixStride = 4;
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
