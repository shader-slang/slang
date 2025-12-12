// optix-ignore-hit.slang
// Tests that any-hit shaders correctly:
// 1. Read payload from registers (register-based approach for small payloads)
// 2. Write payload back before optixIgnoreIntersection() call
// 3. Emit proper OptiX intrinsics
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry anyHitShader
//TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -Xnvrtc -I"./external/optix-dev/include/"

// CHECK: optixGetPayload_0
// CHECK: HitBuffer_insert_0
// CHECK: optixSetPayload_0
// CHECK: optixIgnoreIntersection
// CHECK: optixSetPayload_0

// CHECK-PTX: _optix_get_ray_tmax

struct HitBuffer
{
    float last;
    [mutating]
    void insert(float t) { last = t; }
}

struct RayHit
{
    float t;
    int instanceID;
}

[shader("anyhit")]
void anyHitShader(inout HitBuffer rayHitBuffer)
{
    // Save original value for comparison
    float originalLast = rayHitBuffer.last;

    // Create a hit with current intersection data
    float currentT = RayTCurrent();
    int instanceID = InstanceID();
    RayHit hit = { currentT, instanceID };

    // Modify the inout parameter
    rayHitBuffer.insert(hit.t);

    // CHECK-PTX: _optix_ignore_intersection
    // Early exit - compare against ORIGINAL value, not the modified one
    // This condition can now actually be true (when current hit is closer)
    if (hit.t < originalLast)
        IgnoreHit();

    // This line executes if IgnoreHit() is not called
    // Payload write-back happens here via optixSetPayload
}
