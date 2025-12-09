// Test HitObject API with DXR 1.3 native (dx::HitObject, SM 6.9)

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry rayGenerationMain -stage raygeneration -profile sm_6_9 -capability ser_dxr

// CHECK: dx::HitObject

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

struct MyPayload { float4 color; };

[shader("raygeneration")]
void rayGenerationMain()
{
    int idx = DispatchRaysIndex().x;

    RayDesc ray;
    ray.Origin = float3(idx, 0, 0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0, 1, 0);
    ray.TMax = 1e4f;

    MyPayload payload = { float4(0) };
    uint r = 0;

    // CHECK: dx::HitObject::MakeNop
    HitObject hitFromNop = HitObject::MakeNop();

    // CHECK: .IsNop
    if (hitFromNop.IsNop()) { r += 1; }

    // CHECK: dx::HitObject::Invoke
    HitObject::Invoke(hitFromNop, payload);

    HitObject hitFromTrace = HitObject::TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    // CHECK: .IsHit
    if (hitFromTrace.IsHit())
    {
        // CHECK: .GetRayFlags
        uint rayFlags = hitFromTrace.GetRayFlags();
        r += rayFlags;

        // CHECK: .GetRayTMin
        float tmin = hitFromTrace.GetRayTMin();
        r += uint(tmin);

        // CHECK: .GetRayTCurrent
        float tcurrent = hitFromTrace.GetRayTCurrent();
        r += uint(tcurrent);

        // CHECK: .GetWorldRayOrigin
        float3 worldOrigin = hitFromTrace.GetWorldRayOrigin();
        r += uint(worldOrigin.x);

        // CHECK: .GetWorldRayDirection
        float3 worldDir = hitFromTrace.GetWorldRayDirection();
        r += uint(worldDir.y);

        // CHECK: .GetObjectToWorld3x4
        float3x4 o2w_34 = hitFromTrace.GetObjectToWorld3x4();
        r += uint(o2w_34[0][0]);

        // CHECK: .GetObjectToWorld4x3
        float4x3 o2w_43 = hitFromTrace.GetObjectToWorld4x3();
        r += uint(o2w_43[0][0]);

        // CHECK: .GetWorldToObject3x4
        float3x4 w2o_34 = hitFromTrace.GetWorldToObject3x4();
        r += uint(w2o_34[0][0]);

        // CHECK: .GetWorldToObject4x3
        float4x3 w2o_43 = hitFromTrace.GetWorldToObject4x3();
        r += uint(w2o_43[0][0]);
    }

    outputBuffer[idx] = r;
}
