//DIAGNOSTIC_TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry main -profile spirv_1_4 -capability rayquery -restrictive-capability-check -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -profile sm_6_5 -restrictive-capability-check -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=METAL): -target metal -entry main -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry main -profile spirv_1_4 -capability rayquery -restrictive-capability-check -no-codegen -DUSE_IMPORTED_HELPER
//DIAGNOSTIC_TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -profile sm_6_5 -restrictive-capability-check -no-codegen -DUSE_IMPORTED_HELPER
//DIAGNOSTIC_TEST:SIMPLE(filecheck=METAL): -target metal -entry main -no-codegen -DUSE_IMPORTED_HELPER
//TEST:SIMPLE(filecheck=OK): -target spirv -entry main -profile spirv_1_4 -capability rayquery_opacity_micromap -restrictive-capability-check -no-codegen -DUSE_IMPORTED_HELPER
//TEST:SIMPLE(filecheck=OK): -target hlsl -entry main -profile sm_6_9 -restrictive-capability-check -no-codegen -DUSE_IMPORTED_HELPER
//TEST:SIMPLE(filecheck=OK): -target metal -entry main -no-codegen -DUSE_EMPTY_FEATURES

// Merely declaring the query must propagate the feature's requirement. No query
// operation or target-specific assertion is needed, including when an imported
// alias passes the feature set through a generic helper.
// SPIRV: error[E41013]: entry point uses capabilities not in specified profile
// SPIRV: Missing capabilities are: 'spvRayTracingOpacityMicromapExecutionModeKHR'
// HLSL: error[E41013]: entry point uses capabilities not in specified profile
// HLSL: Missing capabilities are: 'sm_6_9'
// METAL: error[E36107]: unavailable features in entry point
// METAL: RayQueryOpacityMicroMaps
// OK: result code = 0

#ifdef USE_IMPORTED_HELPER
import "ray-query-opacity-micromap-capability-module";
#endif

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
#ifdef USE_IMPORTED_HELPER
    output[0] = declareQuery<MicromapFeatures>();
#else
#ifdef USE_EMPTY_FEATURES
    RayQuery<RAY_FLAG_NONE, RayQueryFeatures<>> query;
#else
    RayQuery<RAY_FLAG_NONE, RayQueryFeatures<RayQueryOpacityMicroMaps>> query;
#endif
    output[0] = 1;
#endif
}
