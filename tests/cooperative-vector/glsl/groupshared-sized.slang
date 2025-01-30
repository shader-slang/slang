//DISABLE_TEST:CROSS_COMPILE: -profile glsl_450+spirv_1_4 -stage compute -entry computeMain -target spirv-assembly

RWStructuredBuffer<float> outputBuffer;

groupshared float buf[100];

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    coopvecNV<float, 8> r;
    coopvecNV<int, 16> v;
    if(dispatchThreadID.x == 0)
    {
        for(int i = 0; i < 100; ++i)
        {
          buf[i] = float(i);
        }
    }
    coopVecLoadNV(v, buf, 0);
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
