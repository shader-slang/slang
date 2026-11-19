//TEST:SIMPLE(filecheck=SPV_ON): -target spirv -entry main -profile spirv_1_4 -capability rayquery_opacity_micromap -restrictive-capability-check
//TEST:SIMPLE(filecheck=SPV_UPGRADE): -target spirv -entry main -profile spirv_1_3 -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPV_OFF): -target spirv -entry main -profile spirv_1_4 -capability rayquery -restrictive-capability-check -DUSE_EMPTY_FEATURES
//TEST:SIMPLE(filecheck=SPV_OFF): -target spirv -entry main -profile spirv_1_4 -capability rayquery -restrictive-capability-check -DUSE_DEFAULT_QUERY
//TEST:SIMPLE(filecheck=HLSL_ON): -target hlsl -entry main -profile sm_6_9 -restrictive-capability-check
//TEST:SIMPLE(filecheck=HLSL_ON): -target hlsl -entry main -profile sm_6_9 -restrictive-capability-check -DREPEAT_MICROMAP_FEATURE
//TEST:SIMPLE(filecheck=HLSL_OFF): -target hlsl -entry main -profile sm_6_5 -restrictive-capability-check -DUSE_EMPTY_FEATURES
//TEST:SIMPLE(filecheck=HLSL_DEFAULT): -target hlsl -entry main -profile sm_6_5 -restrictive-capability-check -DUSE_DEFAULT_QUERY
//TEST:SIMPLE(filecheck=GLSL_ON): -target glsl -entry main -capability rayquery_opacity_micromap
//TEST:SIMPLE(filecheck=GLSL_OFF): -target glsl -entry main -DUSE_EMPTY_FEATURES
//TEST:SIMPLE(filecheck=METAL_OFF): -target metal -entry main -DUSE_EMPTY_FEATURES

// Two opted-in queries need only one entry-point execution mode. The first generic
// argument still controls ray flags, independently of the selected feature types.
// SPV_ON: OpCapability RayTracingOpacityMicromapExecutionModeKHR
// SPV_ON: OpExtension "SPV_KHR_opacity_micromap"
// SPV_ON: OpEntryPoint GLCompute %[[ENTRY:[A-Za-z0-9_]+]] "main"
// SPV_ON-NOT: OpacityMicromapIdKHR
// SPV_ON: OpExecutionModeId %[[ENTRY]] OpacityMicromapIdKHR %[[ENABLED:[A-Za-z0-9_]+]]
// SPV_ON-NOT: OpacityMicromapIdKHR
// SPV_ON: %[[ENABLED]] = OpConstantTrue %bool
// SPV_ON-NOT: OpacityMicromapIdKHR
// SPV_ON: OpRayQueryInitializeKHR
// SPV_ON: OpRayQueryInitializeKHR
// SPV_ON-NOT: OpacityMicromapIdKHR

// The feature requirement must upgrade the binary header before emission.
// SPV_UPGRADE: Version: 1.4
// SPV_UPGRADE: OpCapability RayTracingOpacityMicromapExecutionModeKHR
// SPV_UPGRADE: OpExecutionModeId %{{[A-Za-z0-9_]+}} OpacityMicromapIdKHR

// An empty feature set and the omitted argument must both retain the ordinary path.
// SPV_OFF-NOT: OpacityMicromap
// SPV_OFF-NOT: SPV_KHR_opacity_micromap
// SPV_OFF: OpRayQueryInitializeKHR
// SPV_OFF-NOT: OpacityMicromap
// SPV_OFF-NOT: SPV_KHR_opacity_micromap

// HLSL_ON: RayQuery<2{{[uU]? *, *}}1{{[uU]? *}}>
// HLSL_ON: .TraceRayInline(
// HLSL_OFF-NOT: RayQuery<2{{[uU]? *}},
// HLSL_OFF: RayQuery<2{{[uU]? *}}>
// HLSL_OFF-NOT: RayQuery<2{{[uU]? *}},
// HLSL_DEFAULT-NOT: RayQuery<0{{[uU]? *}},
// HLSL_DEFAULT: RayQuery<0{{[uU]? *}}>
// HLSL_DEFAULT-NOT: RayQuery<0{{[uU]? *}},

// GLSL_ON: #extension GL_EXT_opacity_micromap_ray_query_mode : require
// GLSL_ON: const bool gl_EnableOpacityMicromapEXT = true;
// GLSL_ON-NOT: gl_EnableOpacityMicromapEXT
// GLSL_ON: rayQueryInitializeEXT(
// GLSL_ON-NOT: gl_EnableOpacityMicromapEXT
// GLSL_OFF-NOT: GL_EXT_opacity_micromap_ray_query_mode
// GLSL_OFF-NOT: gl_EnableOpacityMicromapEXT
// GLSL_OFF: rayQueryInitializeEXT(
// GLSL_OFF-NOT: gl_EnableOpacityMicromapEXT

// METAL_OFF: ->reset(

#ifdef USE_EMPTY_FEATURES
typedef RayQueryFeatures<> TestFeatures;
#elif defined(REPEAT_MICROMAP_FEATURE)
// Repeating a feature exercises a nonempty pack tail and still enables only bit 0.
typedef RayQueryFeatures<RayQueryOpacityMicroMaps, RayQueryOpacityMicroMaps> TestFeatures;
#else
typedef RayQueryFeatures<RayQueryOpacityMicroMaps> TestFeatures;
#endif

#ifdef USE_DEFAULT_QUERY
typedef RayQuery TestQuery;
#else
typedef RayQuery<RAY_FLAG_FORCE_NON_OPAQUE, TestFeatures> TestQuery;
#endif

RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0;
    ray.TMax = 100;

    TestQuery first;
    first.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, ray);
    output[0] = first.Proceed();

    TestQuery second;
    second.TraceRayInline(scene, RAY_FLAG_NONE, 0xff, ray);
    output[1] = second.Proceed();
}
