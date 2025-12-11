//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage raygeneration -entry main -capability hlsl_nvapi

RaytracingAccelerationStructure as;

struct testStruct
{
    int data;
}

RWStructuredBuffer<int> outputBuffer1;

void main()
{
    RayDesc ray;
    ray.Origin = float3(0);
    ray.TMin = 1e-6;
    ray.Direction = float3(0,0,1);
    ray.TMax = 100.0f;

    testStruct someInData1;
    someInData1.data = 5;

    // CHECK: TraceRay(
    TraceRay(as,
             1,
             0xff,
             0,
             0,
             2,
             ray,
             someInData1);
    outputBuffer1[0] = outputBuffer1[0]+someInData1.data;

    // Motion blur not supported in HLSL - removed TraceMotionRay test

    // CHECK: NvTraceRayHitObject(
    HitObject::TraceRay(as,
             1,
             0xff,
             0,
             0,
             2,
             ray,
             someInData1);
    outputBuffer1[0] = outputBuffer1[0]+someInData1.data;

    // CHECK: NvInvokeHitObject(
    HitObject hitObject_HitOrMiss;
    HitObject::Invoke(
        as,
        hitObject_HitOrMiss,
        someInData1);
    outputBuffer1[0] = outputBuffer1[0]+someInData1.data;
}