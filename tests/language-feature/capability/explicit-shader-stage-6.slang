//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -allow-glsl -profile sm_5_0

// Our code warns here since we specify profile 'sm_5_0' and a `shader("...")`
// which does not add 'sm_6_3' to our `profile`
//CHECK: warning 36113
//CHECK: warning 41012 
struct RayPayload
{
    uint data;
};

uniform RaytracingAccelerationStructure sceneBVH;
RWStructuredBuffer<uint> outputBuffer;
[shader("_raygen")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0,0,0);
    ray.Direction = float3(0,0,0);
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = {};
    TraceRay(sceneBVH, RAY_FLAG_NONE, ~0, 0, 0, 0, ray, payload);

    outputBuffer[0] = payload.data;
}
