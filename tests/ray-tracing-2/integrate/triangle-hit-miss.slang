//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry rayGenerationMain -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry rayGenerationMain -stage raygeneration -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry rayGenerationMain -stage raygeneration -target metal
//TEST:SIMPLE(filecheck=CUDA): -experimental-feature -entry rayGenerationMain -stage raygeneration -target cuda

//HLSL: [shader("raygeneration")]void rayGenerationMain()
//HLSL: [shader("miss")]void RuntimeMiss(
//HLSL: [shader("closesthit")]void RuntimeClosestHit(
// SPIR-V keeps the default target entry-point name even though the portable source name avoids
// CUDA's reserved `main` identifier.
//SPIRV: OpEntryPoint RayGenerationKHR {{.*}} "main"
//SPIRV: OpEntryPoint MissKHR {{.*}} "RuntimeMiss"
//SPIRV: OpEntryPoint ClosestHitKHR {{.*}} "RuntimeClosestHit"
//METAL-DAG: thread_position_in_grid
//METAL-DAG: threads_per_grid
//CUDA-DAG: optixTrace(
//CUDA-DAG: __raygen__rayGenerationMain
//CUDA-DAG: __miss__RuntimeMiss
//CUDA-DAG: __closesthit__RuntimeClosestHit

// The runtime shader is also the first complete structural pipeline integration case.
#include "../runtime/shaders/triangle-hit-miss.slang"
