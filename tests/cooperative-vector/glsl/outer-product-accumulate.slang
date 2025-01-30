//DISABLE_TEST:CROSS_COMPILE: -profile glsl_450+spirv_1_4 -stage compute -entry computeMain -target spirv-assembly -emit-spirv-via-glsl

RWByteAddressBuffer buf;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    coopvecNV<float, 8> op0 = coopvecNV<float, 8>(1);
    coopvecNV<float, 16> op1 = coopvecNV<float, 16>(2);

    coopVecOuterProductAccumulateNV(
        op0,
        op1,
        buf,
        0, // firstElement
        4, // matrixStride
        gl_CooperativeVectorMatrixLayoutRowMajorNV,
        gl_ComponentTypeFloat32NV);
}
