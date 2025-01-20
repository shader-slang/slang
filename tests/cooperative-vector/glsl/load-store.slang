//DISABLE_TEST:CROSS_COMPILE: -profile glsl_450+spirv_1_4 -stage compute -entry computeMain -target spirv-assembly

RWStructuredBuffer<float> rwBuf;
StructuredBuffer<float> buf;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    coopvecNV<float, 8> r;
    coopVecLoadNV(r, rwBuf, 0);
    coopVecLoadNV(r, buf, 1);
    coopVecStoreNV(r, rwBuf, 2);
}
