//TEST:SIMPLE(filecheck=SPV_ON): -target spirv -entry declared -profile spirv_1_3 -I tests/diagnostics
//TEST:SIMPLE(filecheck=SPV_ON): -target spirv -entry imported -profile spirv_1_4 -capability rayquery_opacity_micromap -I tests/diagnostics
//TEST:SIMPLE(filecheck=SPV_ON): -target spirv -entry genericMain -specialize MicromapFeatures -profile spirv_1_4 -capability rayquery_opacity_micromap -I tests/diagnostics
//TEST:SIMPLE(filecheck=SPV_OFF): -target spirv -entry genericMain -specialize DefaultFeatures -profile spirv_1_4 -capability rayquery_opacity_micromap -I tests/diagnostics
//TEST:SIMPLE(filecheck=SPV_OFF): -target spirv -entry ordinary -profile spirv_1_4 -capability rayquery_opacity_micromap -I tests/diagnostics
//TEST:SIMPLE(filecheck=COMBINED): -target spirv -entry combined -profile spirv_1_4 -capability rayquery_opacity_micromap -capability spvShader64BitIndexingEXT -I tests/diagnostics
//TEST:SIMPLE(filecheck=RAYTRACE): -target spirv -entry declaredRayGeneration -profile spirv_1_4 -capability rayquery_opacity_micromap -I tests/diagnostics
//TEST:SIMPLE(filecheck=RAYTRACE_NO_QUERY): -target spirv -entry declaredRayGeneration -profile spirv_1_4 -capability rayquery_opacity_micromap -I tests/diagnostics
//TEST:SIMPLE(filecheck=GLSL_ON): -target glsl -entry declared -capability rayquery_opacity_micromap -I tests/diagnostics
//TEST:SIMPLE(filecheck=GLSL_OFF): -target glsl -entry ordinary -capability rayquery_opacity_micromap -I tests/diagnostics
//TEST:COMPILE: tests/spirv/ray-query-opacity-micromap-feature-capability.slang -I tests/diagnostics -entry declared -o tests/spirv/ray-query-opacity-micromap-feature-capability.slang-module
//TEST:SIMPLE_EX(filecheck=SPV_ON): -I tests/diagnostics tests/spirv/ray-query-opacity-micromap-feature-capability.slang-module -target spirv -entry declared -profile spirv_1_4 -capability rayquery_opacity_micromap
//TEST:SIMPLE_EX(filecheck=SPV_ON): -I tests/diagnostics tests/spirv/ray-query-opacity-micromap-feature-capability.slang-module -target spirv -entry genericMain -specialize MicromapFeatures -profile spirv_1_4 -capability rayquery_opacity_micromap
//TEST:SIMPLE_EX(filecheck=SPV_OFF): -I tests/diagnostics tests/spirv/ray-query-opacity-micromap-feature-capability.slang-module -target spirv -entry genericMain -specialize DefaultFeatures -profile spirv_1_4 -capability rayquery_opacity_micromap
//TEST:SIMPLE_EX(filecheck=SPV_OFF): -I tests/diagnostics tests/spirv/ray-query-opacity-micromap-feature-capability.slang-module -target spirv -entry ordinary -profile spirv_1_4 -capability rayquery_opacity_micromap

// Selecting the feature type requires the execution mode even when no query
// operation survives. The same requirement must survive imported generic helpers,
// entry-point specialization, and serialized modules.
// SPV_ON: Version: 1.4
// SPV_ON: OpCapability RayTracingOpacityMicromapExecutionModeKHR
// SPV_ON-DAG: OpExtension "SPV_KHR_opacity_micromap"
// SPV_ON-DAG: OpExtension "SPV_KHR_ray_query"
// SPV_ON: OpEntryPoint GLCompute %[[ENTRY:[A-Za-z0-9_]+]]
// SPV_ON: OpExecutionModeId %[[ENTRY]] OpacityMicromapIdKHR %[[ENABLED:[A-Za-z0-9_]+]]
// SPV_ON-NOT: OpacityMicromapIdKHR
// SPV_ON: %[[ENABLED]] = OpConstantTrue %bool
// SPV_ON-NOT: OpRayQueryInitializeKHR
// SPV_ON: OpFunctionEnd
// SPV_ON-NOT: OpRayQueryInitializeKHR

// Allowing the capability on the target does not opt ordinary entry points in.
// SPV_OFF-NOT: OpacityMicromap
// SPV_OFF-NOT: SPV_KHR_opacity_micromap
// SPV_OFF: OpFunctionEnd
// SPV_OFF-NOT: OpacityMicromap
// SPV_OFF-NOT: SPV_KHR_opacity_micromap

// GLSL_ON-DAG: #extension GL_EXT_ray_query : require
// GLSL_ON-DAG: #extension GL_EXT_opacity_micromap_ray_query_mode : require
// GLSL_ON: const bool gl_EnableOpacityMicromapEXT = true;
// GLSL_ON-NOT: rayQueryInitializeEXT
// GLSL_ON: void main(
// GLSL_ON-NOT: rayQueryInitializeEXT
// GLSL_OFF-NOT: GL_EXT_opacity_micromap_ray_query_mode
// GLSL_OFF-NOT: gl_EnableOpacityMicromapEXT
// GLSL_OFF: void main(
// GLSL_OFF-NOT: gl_EnableOpacityMicromapEXT

// Ray-generation entry points already require ray tracing, which also satisfies
// the SPIR-V micromap extension dependency without adding the ray-query extension.
// RAYTRACE-DAG: OpExtension "SPV_KHR_ray_tracing"
// RAYTRACE-DAG: OpExtension "SPV_KHR_opacity_micromap"
// RAYTRACE: OpEntryPoint RayGenerationKHR %declaredRayGeneration
// RAYTRACE: OpExecutionModeId %declaredRayGeneration OpacityMicromapIdKHR %[[ENABLED:[A-Za-z0-9_]+]]
// RAYTRACE: %[[ENABLED]] = OpConstantTrue %bool
// RAYTRACE-NOT: OpRayQueryInitializeKHR
// RAYTRACE_NO_QUERY-NOT: SPV_KHR_ray_query
// RAYTRACE_NO_QUERY: OpEntryPoint RayGenerationKHR %declaredRayGeneration

// Both capability-driven modes must survive on the same entry point.
// COMBINED-DAG: OpCapability RayTracingOpacityMicromapExecutionModeKHR
// COMBINED-DAG: OpCapability Shader64BitIndexingEXT
// COMBINED-DAG: OpExecutionModeId %combined OpacityMicromapIdKHR %[[ENABLED:[A-Za-z0-9_]+]]
// COMBINED-DAG: OpExecutionMode %combined Shader64BitIndexingEXT
// COMBINED-NOT: OpExecutionModeId %combined OpacityMicromapIdKHR
// COMBINED-NOT: OpExecutionMode %combined Shader64BitIndexingEXT
// COMBINED: %[[ENABLED]] = OpConstantTrue %bool
// COMBINED-NOT: OpExecutionModeId %combined OpacityMicromapIdKHR
// COMBINED-NOT: OpExecutionMode %combined Shader64BitIndexingEXT

import "ray-query-opacity-micromap-capability-module";

typealias DefaultFeatures = RayQueryFeatures<>;

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void declared()
{
    RayQuery<RAY_FLAG_NONE, MicromapFeatures> query;
    output[0] = 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void imported()
{
    output[1] = declareQuery<MicromapFeatures>();
}

[shader("compute")]
[numthreads(1, 1, 1)]
void genericMain<Features : IRayQueryFeature>()
{
    RayQuery<RAY_FLAG_NONE, Features> query;
    output[2] = 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void ordinary()
{
    RayQuery query;
    output[3] = 1;
}

[Shader64BitIndexing]
[require(rayquery_opacity_micromap)]
[shader("compute")]
[numthreads(1, 1, 1)]
void combined()
{
    RayQuery<RAY_FLAG_NONE, MicromapFeatures> query;
    output[4] = 1;
}

[shader("raygeneration")]
void declaredRayGeneration()
{
    RayQuery<RAY_FLAG_NONE, MicromapFeatures> query;
    output[5] = 1;
}
