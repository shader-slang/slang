//DISABLE_TEST:CROSS_COMPILE: -profile glsl_450+spirv_1_4 -stage compute -entry computeMain -target spirv-assembly

// Disabled because we don't output the extension because although the
// coopvecNV type is used, it isn't *declared* here (and we don't attach
// requirements to the add operations);

RWStructuredBuffer<float> outputBuffer;

StructuredBuffer<float> buf;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    coopvecNV<float, 8> r;
    coopvecNV<float, 8> v = coopvecNV<float, 8>(1);
    r = v + v;
    outputBuffer[dispatchThreadID.x] = r[0];
}
