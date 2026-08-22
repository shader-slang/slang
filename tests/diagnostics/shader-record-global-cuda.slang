// A shader-record-attributed global constant buffer is not routed through the OptiX
// shader binding table on CUDA/PTX; it must be diagnosed rather than silently folded
// into the launch-parameter struct. See shader-slang/slang#12628.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -stage closesthit -entry chsMain -target cuda
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -stage closesthit -entry chsMain -target ptx

struct SbtData
{
    float4 color;
};

layout(shaderRecordEXT) ConstantBuffer<SbtData> gSbtRecordData;

RWStructuredBuffer<float4> gOut;

[shader("closesthit")]
void chsMain(inout float4 payload, in BuiltInTriangleIntersectionAttributes attr)
{
    payload = gSbtRecordData.color;
    gOut[0] = payload;
}

//CHECK: shader record global not supported
//CHECK: entry point 'uniform' parameter
