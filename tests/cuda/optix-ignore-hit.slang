// optix-ignore-hit.slang
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry anyHitShader
//CHECK: HitBuffer_insert_0((HitBuffer_0 *)getOptiXRayPayloadPtr(), hit_0.t_0);
//CHECK: optixIgnoreIntersection


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
    // Create a hit with current intersection data
    float currentT = RayTCurrent();
    int instanceID = InstanceID();
    RayHit hit = { currentT, instanceID };
    
    // Modify the inout parameter
    rayHitBuffer.insert(hit.t);
    
    // Early exit - should not lose the modification to inout rayHitBuffer
    if (hit.t < rayHitBuffer.last)
        IgnoreHit();
    
    // This line should never execute if IgnoreHit() is called,
    // but parameter changes should still propagate back
}
