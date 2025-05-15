//TEST:SIMPLE(filecheck=WITHOUT_RAY_ACCELERATION_SUPPORT): -target spirv -stage compute -entry computeMain -DWITHOUT_RAY_ACCELERATION_SUPPORT
//TEST:SIMPLE(filecheck=WITH_RAY_ACCELERATION_SUPPORT): -target spirv -stage compute -entry computeMain -DWITH_RAY_ACCELERATION_SUPPORT

// To intentionally fill up binding slots
[[vk::binding(0, 1)]]
RWTexture1D<float> t1;

[[vk::binding(0, 2)]]
RWTexture1D<float> t2;

[[vk::binding(0, 4)]]
RWTexture1D<float> t3;

[[vk::binding(1, 4)]]
Texture1D<float> t4;

//WITHOUT_RAY_ACCELERATION_SUPPORT-DAG: OpDecorate %__slang_resource_heap{{.*}} Binding 0
//WITHOUT_RAY_ACCELERATION_SUPPORT-DAG: OpDecorate %__slang_resource_heap{{.*}} Binding 1
//WITHOUT_RAY_ACCELERATION_SUPPORT-DAG: OpDecorate %__slang_resource_heap{{.*}} Binding 2
//WITHOUT_RAY_ACCELERATION_SUPPORT-DAG: OpDecorate %__slang_resource_heap{{.*}} Binding 3
//WITHOUT_RAY_ACCELERATION_SUPPORT-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 4
//WITHOUT_RAY_ACCELERATION_SUPPORT-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 5
//WITHOUT_RAY_ACCELERATION_SUPPORT-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 6
//WITHOUT_RAY_ACCELERATION_SUPPORT-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 7
//WITHOUT_RAY_ACCELERATION_SUPPORT-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 8
//WITHOUT_RAY_ACCELERATION_SUPPORT-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 9
#ifdef WITHOUT_RAY_ACCELERATION_SUPPORT
export T getDescriptorFromHandle<T:IOpaqueDescriptor>(DescriptorHandle<T> handleValue)
{
    return defaultGetDescriptorFromHandle(handleValue, BindlessDescriptorOptions.VkMutable);
}
#endif

//WITH_RAY_ACCELERATION_SUPPORT-DAG: OpDecorate %__slang_resource_heap{{.*}} Binding 0
//WITH_RAY_ACCELERATION_SUPPORT-DAG: OpDecorate %__slang_resource_heap{{.*}} Binding 1
//WITH_RAY_ACCELERATION_SUPPORT-DAG: OpDecorate %__slang_resource_heap{{.*}} Binding 2
//WITH_RAY_ACCELERATION_SUPPORT-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 3
//WITH_RAY_ACCELERATION_SUPPORT-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 4
//WITH_RAY_ACCELERATION_SUPPORT-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 5
//WITH_RAY_ACCELERATION_SUPPORT-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 6
//WITH_RAY_ACCELERATION_SUPPORT-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 7
//WITH_RAY_ACCELERATION_SUPPORT-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 8
//WITH_RAY_ACCELERATION_SUPPORT-NOT: OpDecorate %__slang_resource_heap{{.*}} Binding 9
#ifdef WITH_RAY_ACCELERATION_SUPPORT
export T getDescriptorFromHandle<T:IOpaqueDescriptor>(DescriptorHandle<T> handleValue)
{
    return defaultGetDescriptorFromHandle(handleValue, BindlessDescriptorOptions.VkMutableRayAcceleration);
}
#endif

uniform SamplerState.Handle sampler;
uniform Sampler1DShadow.Handle combinedSampler;
uniform Texture1D<float>.Handle texture;
uniform RWTexture1D<float>.Handle rwTexture1;
uniform RWTexture2D<float>.Handle rwTexture2;
uniform Buffer<float>.Handle texelBuffer;
uniform RWBuffer<float>.Handle rwTexelBuffer;

struct Data
{
    float v;
}
uniform ConstantBuffer<Data>.Handle buffer1;
uniform StructuredBuffer<float>.Handle buffer2;
uniform RWStructuredBuffer<float>.Handle rwBuffer;
uniform RaytracingAccelerationStructure.Handle rayAcceleration;


[shader("compute")]
void computeMain()
{

    t1[0] = t2[0] + t2[0] + t4[0];
    t1[2] = t4.Sample(sampler, 0);
    t1[8] = combinedSampler.Sample(0);
    t1[0] = texture[0];
    t1[11] = rwTexture1[0];
    t1[12] = rwTexture2[0];
    t1[10] = texelBuffer[0];
    t1[9] = rwTexelBuffer[0];
    t1[4] = (*buffer1).v;
    t1[6] = buffer2[0];
    t1[0] += rwBuffer[0];

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

}