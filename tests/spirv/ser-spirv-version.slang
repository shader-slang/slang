// Test for issue #12097: the shader-invocation-reorder (SER) capabilities must be
// usable at SPIR-V 1.4 (with a physical-storage-buffer extension declared) rather than
// forcing the module header up to 1.5. Higher profiles keep their own version.

// A SER capability at -profile spirv_1_4 must stay at 1.4 and declare the KHR
// physical-storage-buffer extension so the module is valid at 1.4.
//TEST:SIMPLE(filecheck=NV14): -target spirv-asm -emit-spirv-directly -profile spirv_1_4 -capability spvShaderInvocationReorderNV -entry main -stage raygeneration
//TEST:SIMPLE(filecheck=EXT14): -target spirv-asm -emit-spirv-directly -profile spirv_1_4 -capability spvShaderInvocationReorderEXT -entry main -stage raygeneration

// A higher profile keeps its core version and must NOT gain a spurious PSB extension
// (physical_storage_buffer is core from 1.5).
//TEST:SIMPLE(filecheck=NV15): -target spirv-asm -emit-spirv-directly -profile spirv_1_5 -capability spvShaderInvocationReorderNV -entry main -stage raygeneration
//TEST:SIMPLE(filecheck=EXT15): -target spirv-asm -emit-spirv-directly -profile spirv_1_5 -capability spvShaderInvocationReorderEXT -entry main -stage raygeneration

// Also compile to a binary SPIR-V module so CI's SLANG_RUN_SPIRV_VALIDATION=1 runs
// spirv-val against the real 1.4 module — a clean slangc exit alone is insufficient.
//TEST:SIMPLE(filecheck=NV14): -target spirv -emit-spirv-directly -profile spirv_1_4 -capability spvShaderInvocationReorderNV -entry main -stage raygeneration
//TEST:SIMPLE(filecheck=EXT14): -target spirv -emit-spirv-directly -profile spirv_1_4 -capability spvShaderInvocationReorderEXT -entry main -stage raygeneration

// The 1.4 module keeps the logical addressing model; the PSB dependency needs the
// extension declared, not PhysicalStorageBuffer64 addressing.
// NV14: Version: 1.4
// NV14-DAG: OpCapability ShaderInvocationReorderNV
// NV14-DAG: OpExtension "SPV_KHR_physical_storage_buffer"
// NV14-DAG: OpExtension "SPV_NV_shader_invocation_reorder"
// NV14-DAG: OpMemoryModel Logical GLSL450

// EXT14: Version: 1.4
// EXT14-DAG: OpCapability ShaderInvocationReorderEXT
// EXT14-DAG: OpExtension "SPV_KHR_physical_storage_buffer"
// EXT14-DAG: OpExtension "SPV_EXT_shader_invocation_reorder"
// EXT14-DAG: OpMemoryModel Logical GLSL450

// NV15: Version: 1.5
// NV15-NOT: OpExtension "SPV_KHR_physical_storage_buffer"

// EXT15: Version: 1.5
// EXT15-NOT: OpExtension "SPV_KHR_physical_storage_buffer"

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
