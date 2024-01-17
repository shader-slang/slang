// closesthit.slang

// Note: explicitly requesting `spirv_1_4` as part of the compiler
// options is for the benefit of the glslang pass-through path,
// where we cannot infer the required SPIR-V version from the code
// (and glslang doesn't include any such inference ability).

//TEST:SIMPLE(filecheck=CHECK): -profile glsl_460+spirv_1_4 -stage anyhit -entry main -target spirv-assembly

struct SphereHitAttributes
{
    float3 normal;
};


struct ShadowRay
{
    float4 hitDistance;
};

struct Params
{
    Texture2D<float>    alphaMap;
    SamplerState        sampler;
    int                 mode;
}
ParameterBlock<Params> gParams;

void main(
    SphereHitAttributes attributes,
    in out ShadowRay    ioPayload)
{
    if(gParams.mode != 0)
    {
        float val = gParams.alphaMap.SampleLevel(
            gParams.sampler,
            attributes.normal.xy, 0);
        if(val > 0)
        {
            AcceptHitAndEndSearch();
        }
        else
        {
            IgnoreHit();
        }
    }

    uint index = 0U;
	HitTriangleVertexPosition(index);

	index = 1U;
	HitTriangleVertexPosition(index);

	index = 2U;
	HitTriangleVertexPosition(index);
}

// CHECK: OpCapability RayTracing
// CHECK: OpCapability RayTracingPositionFetchKHR
// CHECK: OpEntryPoint AnyHitNV %main "main"
// CHECK: OpDecorate %{{.*}} BuiltIn HitTriangleVertexPositionsKHR
// CHECK: %_ptr_HitAttributeNV_SphereHitAttributes_0 = OpTypePointer HitAttributeNV %SphereHitAttributes_0
// CHECK: %_S{{.*}} = OpVariable %_ptr_HitAttributeNV_SphereHitAttributes_0 HitAttributeNV
// CHECK: %{{.*}} = OpAccessChain %_ptr_HitAttributeNV_v3float %_S{{.*}} %int_0
// CHECK: OpTerminateRayKHR
// CHECK: OpIgnoreIntersectionKHR
// CHECK: %{{.*}} = OpAccessChain %_ptr_Input_v3float %{{.*}} %{{u?}}int_0
// CHECK: %{{.*}} = OpAccessChain %_ptr_Input_v3float %{{.*}} %{{u?}}int_1
// CHECK: %{{.*}} = OpAccessChain %_ptr_Input_v3float %{{.*}} %{{u?}}int_2
