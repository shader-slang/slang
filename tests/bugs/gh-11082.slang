// Test for issue #11082: implicit SER capability upgrades should consistently use EXT.

//TEST:SIMPLE(filecheck=IMPLICIT): -target spirv-asm -emit-spirv-directly -profile spirv_1_6 -entry main -stage raygeneration
//TEST:SIMPLE(filecheck=EXT_RESTRICTIVE): -target spirv-asm -emit-spirv-directly -profile spirv_1_6 -capability spvShaderInvocationReorderEXT -restrictive-capability-check -entry main -stage raygeneration

// IMPLICIT: warning[E41012]
// IMPLICIT: spvShaderInvocationReorderEXT
// IMPLICIT-NOT: spvShaderInvocationReorderNV
// IMPLICIT-NOT: ShaderInvocationReorderNV
// IMPLICIT-NOT: SPV_NV_shader_invocation_reorder
// IMPLICIT: OpCapability ShaderInvocationReorderEXT
// IMPLICIT: OpExtension "SPV_EXT_shader_invocation_reorder"
// IMPLICIT-NOT: OpTypeHitObjectNV
// IMPLICIT: OpTypeHitObjectEXT
// IMPLICIT-NOT: HitObjectAttributeNV
// IMPLICIT: HitObjectAttributeEXT
// IMPLICIT-NOT: OpHitObjectTraceRayNV
// IMPLICIT: OpHitObjectTraceRayEXT
// IMPLICIT-NOT: OpHitObjectGetAttributesNV
// IMPLICIT: OpHitObjectGetAttributesEXT

// EXT_RESTRICTIVE-NOT: error[E41013]
// EXT_RESTRICTIVE-NOT: warning[E41012]
// EXT_RESTRICTIVE-NOT: ShaderInvocationReorderNV
// EXT_RESTRICTIVE-NOT: SPV_NV_shader_invocation_reorder
// EXT_RESTRICTIVE: OpCapability ShaderInvocationReorderEXT
// EXT_RESTRICTIVE: OpExtension "SPV_EXT_shader_invocation_reorder"
// EXT_RESTRICTIVE-NOT: OpTypeHitObjectNV
// EXT_RESTRICTIVE: OpTypeHitObjectEXT
// EXT_RESTRICTIVE-NOT: HitObjectAttributeNV
// EXT_RESTRICTIVE: HitObjectAttributeEXT
// EXT_RESTRICTIVE-NOT: OpHitObjectTraceRayNV
// EXT_RESTRICTIVE: OpHitObjectTraceRayEXT
// EXT_RESTRICTIVE-NOT: OpHitObjectGetAttributesNV
// EXT_RESTRICTIVE: OpHitObjectGetAttributesEXT

RaytracingAccelerationStructure tlas;
RWTexture2D<float> outImage;

struct Payload
{
    uint foo;
};

struct Attributes
{
    float2 barycentrics;
};

[shader("raygeneration")]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0.0);
    ray.Direction = float3(1.0);
    ray.TMin = 0.0;
    ray.TMax = 1e30;

    Payload payload;
    HitObject hitObj = HitObject::TraceRay(tlas, 0, 0xFF, 0, 0, 0, ray, payload);
    if (hitObj.IsHit())
    {
        Attributes attributes = hitObj.GetAttributes<Attributes>();
        outImage[DispatchRaysIndex().xy] = 1.0f + attributes.barycentrics.x;
    }
}
