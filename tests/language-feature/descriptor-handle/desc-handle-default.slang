//TEST:SIMPLE(filecheck=SAMPLER): -target spirv -stage compute -entry computeMain -DSAMPLER
//TEST:SIMPLE(filecheck=COMBINED_IMAGE_SAMPLER): -target spirv -stage compute -entry computeMain -DCOMBINED_IMAGE_SAMPLER
//TEST:SIMPLE(filecheck=SAMPLED_IMAGE): -target spirv -stage compute -entry computeMain -DSAMPLED_IMAGE
//TEST:SIMPLE(filecheck=STORAGE_IMAGE): -target spirv -stage compute -entry computeMain -DSTORAGE_IMAGE
//TEST:SIMPLE(filecheck=UNIFORM_TEXEL_BUFFER): -target spirv -stage compute -entry computeMain -DUNIFORM_TEXEL_BUFFER
//TEST:SIMPLE(filecheck=STORAGE_TEXEL_BUFFER): -target spirv -stage compute -entry computeMain -DSTORAGE_TEXEL_BUFFER
//TEST:SIMPLE(filecheck=UNIFORM_BUFFER): -target spirv -stage compute -entry computeMain -DUNIFORM_BUFFER
//TEST:SIMPLE(filecheck=STORAGE_BUFFER): -target spirv -stage compute -entry computeMain -DSTORAGE_BUFFER
//TEST:SIMPLE(filecheck=ACCELERATION_STRUCTURE): -target spirv -stage compute -entry computeMain -DACCELERATION_STRUCTURE
//TEST:SIMPLE(filecheck=SAMPLED_IMAGE): -target spirv -stage compute -entry computeMain -DSAMPLED_IMAGE
//TEST:SIMPLE(filecheck=MIX): -target spirv -stage compute -entry computeMain -DSAMPLER -DSTORAGE_TEXEL_BUFFER -DUNIFORM_BUFFER -DACCELERATION_STRUCTURE


export T getDescriptorFromHandle<T : IOpaqueDescriptor>(DescriptorHandle<T> handleValue)
{
    return defaultGetDescriptorFromHandle(handleValue, BindlessDescriptorOptions.None);
}

// To intentionally fill up binding slots
[[vk::binding(0, 1)]]
RWTexture1D<float> t1;

[[vk::binding(0, 2)]]
RWTexture1D<float> t2;

[[vk::binding(0, 4)]]
RWTexture1D<float> t3;

[[vk::binding(1, 4)]]
Texture1D<float> t4;

//MIX-DAG: OpDecorate %__slang_resource_heap{{.*}} Binding 0
//MIX-DAG: OpDecorate %__slang_resource_heap{{.*}} Binding 5
//MIX-DAG: OpDecorate %__slang_resource_heap{{.*}} Binding 6

//MIX-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 1
//MIX-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 2
//MIX-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 3
//MIX-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 4
//MIX-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 7
//MIX-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 8

#ifdef SAMPLER
//SAMPLER: OpDecorate %__slang_resource_heap{{.*}} Binding 0
//SAMPLER-NEXT: OpDecorate %__slang_resource_heap{{.*}} DescriptorSet 3

//SAMPLER-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 1
//SAMPLER-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 2
//SAMPLER-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 3
//SAMPLER-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 4
//SAMPLER-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 5
//SAMPLER-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 6
//SAMPLER-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 7
//SAMPLER-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 8
uniform SamplerState.Handle sampler;
#endif

#ifdef COMBINED_IMAGE_SAMPLER
//COMBINED_IMAGE_SAMPLER: OpDecorate %__slang_resource_heap{{.*}} Binding 1
//COMBINED_IMAGE_SAMPLER-NEXT: OpDecorate %__slang_resource_heap{{.*}} DescriptorSet 3
uniform Sampler1DShadow.Handle combinedSampler;
#endif

#ifdef SAMPLED_IMAGE
//SAMPLED_IMAGE: OpDecorate %__slang_resource_heap{{.*}} Binding 2
//SAMPLED_IMAGE-NEXT: OpDecorate %__slang_resource_heap{{.*}} DescriptorSet 3
uniform Texture1D<float>.Handle texture;
#endif

#ifdef STORAGE_IMAGE
//STORAGE_IMAGE: OpDecorate %__slang_resource_heap{{.*}} Binding 3
//STORAGE_IMAGE-NEXT: OpDecorate %__slang_resource_heap{{.*}} DescriptorSet 3
uniform RWTexture1D<float>.Handle rwTexture1;
uniform RWTexture2D<float>.Handle rwTexture2;
#endif

#ifdef UNIFORM_TEXEL_BUFFER
//UNIFORM_TEXEL_BUFFER: OpDecorate %__slang_resource_heap{{.*}} Binding 4
//UNIFORM_TEXEL_BUFFER-NEXT: OpDecorate %__slang_resource_heap{{.*}} DescriptorSet 3
uniform Buffer<float>.Handle texelBuffer;
#endif

#ifdef STORAGE_TEXEL_BUFFER
//STORAGE_TEXEL_BUFFER: OpDecorate %__slang_resource_heap{{.*}} Binding 5
//STORAGE_TEXEL_BUFFER-NEXT: OpDecorate %__slang_resource_heap{{.*}} DescriptorSet 3
uniform RWBuffer<float>.Handle rwTexelBuffer;
#endif

#ifdef UNIFORM_BUFFER
//UNIFORM_BUFFER: OpDecorate %__slang_resource_heap{{.*}} Binding 6
//UNIFORM_BUFFER-NEXT: OpDecorate %__slang_resource_heap{{.*}} DescriptorSet 3
struct Data
{
    float v;
}
uniform ConstantBuffer<Data>.Handle buffer1;
uniform StructuredBuffer<float>.Handle buffer2;
#endif

#ifdef STORAGE_BUFFER
//STORAGE_BUFFER: OpDecorate %__slang_resource_heap{{.*}} Binding 7
//STORAGE_BUFFER-NEXT: OpDecorate %__slang_resource_heap{{.*}} DescriptorSet 3
uniform RWStructuredBuffer<float>.Handle rwBuffer;
#endif

#ifdef ACCELERATION_STRUCTURE
//ACCELERATION_STRUCTURE-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 8
//ACCELERATION_STRUCTURE: OpConvertUToAccelerationStructureKHR
uniform RaytracingAccelerationStructure.Handle rayAcceleration;
#endif

[shader("compute")]
void computeMain()
{

    t1[0] = t2[0] + t2[0] + t4[0];
    
#ifdef SAMPLER
    t1[2] = t4.Sample(sampler, 0);
#endif

#ifdef COMBINED_IMAGE_SAMPLER
    t1[8] = combinedSampler.Sample(0);
#endif

#ifdef SAMPLED_IMAGE
    t1[0] = texture[0];
#endif

#ifdef STORAGE_IMAGE
    t1[11] = rwTexture1[0];
    t1[12] = rwTexture2[0];
#endif

#ifdef UNIFORM_TEXEL_BUFFER
    t1[10] = texelBuffer[0];
#endif

#ifdef STORAGE_TEXEL_BUFFER
    t1[9] = rwTexelBuffer[0];
#endif

#ifdef UNIFORM_BUFFER
    t1[4] = (*buffer1).v;
    t1[6] = buffer2[0];
#endif

#ifdef STORAGE_BUFFER
    t1[0] += rwBuffer[0];
#endif
  
#ifdef ACCELERATION_STRUCTURE
    RayDesc ray;
    ray.Origin = float3(0.1f, 0.1f, 0.0f);
    ray.Direction = float3(0.0f, 0.0f, 1.0f);
    ray.TMin = 0.0f;
    ray.TMax = 100.0f;
    RayQuery<RAY_FLAG_FORCE_NON_OPAQUE> rq;
    rq.TraceRayInline(rayAcceleration, RAY_FLAG_FORCE_NON_OPAQUE, 0xff, ray);
    bool proceed = rq.Proceed();
    rq.CommitNonOpaqueTriangleHit();
    rq.Abort();
    t1[13] = (float)rq.RayFlags();
#endif
}