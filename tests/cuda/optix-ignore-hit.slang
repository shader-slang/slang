// optix-ignore-hit.slang
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry anyHitShader
//TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -Xnvrtc -I"./external/optix-dev/include/"
//CHECK: optixGetPayload_0
//CHECK: HitBuffer_insert_0
//CHECK: optixSetPayload_0
//CHECK: optixIgnoreIntersection

//CHECK-PTX: _optix_get_ray_tmax
//CHECK-PTX: _optix_set_payload
//CHECK-PTX: _optix_ignore_intersection

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
    // Save original value before modification
    float originalLast = rayHitBuffer.last;

    // Create a hit with current intersection data
    float currentT = RayTCurrent();
    int instanceID = InstanceID();
    RayHit hit = { currentT, instanceID };

    // Modify the inout parameter
    rayHitBuffer.insert(hit.t);

    // Early exit - compare against original value
    if (hit.t < originalLast)
        IgnoreHit();
    
    // This line should never execute if IgnoreHit() is called,
    // but parameter changes should still propagate back
}
