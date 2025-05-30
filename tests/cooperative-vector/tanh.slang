//TEST:SIMPLE(filecheck=CHECK): -target cuda

// CHECK: OptixCoopVec
// CHECK: optixCoopVecTanh


//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4),name=input
ByteAddressBuffer input;

struct RayPayload
{
    float4 color;
    float2x4 lssData;
    bool isSphere;
    bool isLss;
};


[numthreads(1, 1, 1)]
[shader("closesthit")]
void closestHitShaderLss(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    CoopVec<float, 4> vec = coopVecLoad<4, float>(input);

    CoopVec<float, 4> result = tanh(vec);

    for(int i = 0; i < result.getCount(); ++i)
        outputBuffer[i] = result[i];;
}
