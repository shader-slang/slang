//TEST:SIMPLE(filecheck=HLSL): -experimental-feature -entry main -stage raygeneration -target hlsl -profile sm_6_5
//TEST:SIMPLE(filecheck=SPIRV): -experimental-feature -entry main -stage raygeneration -target spirv-assembly -emit-spirv-directly
//TEST:SIMPLE(filecheck=METAL): -experimental-feature -entry main -stage raygeneration -target metal

//HLSL: [shader("raygeneration")]void main()
//HLSL: [shader("miss")]void RuntimeMiss(
//HLSL: [shader("closesthit")]void RuntimeClosestHit(
//SPIRV: OpEntryPoint RayGenerationKHR {{.*}} "main"
//SPIRV: OpEntryPoint MissKHR {{.*}} "RuntimeMiss"
//SPIRV: OpEntryPoint ClosestHitKHR {{.*}} "RuntimeClosestHit"
//METAL-DAG: thread_position_in_grid
//METAL-DAG: threads_per_grid

// The runtime shader is also the first complete structural pipeline integration case.
#include "../runtime/shaders/triangle-hit-miss.slang"
