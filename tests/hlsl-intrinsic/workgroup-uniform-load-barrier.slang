// `workgroupUniformLoad` has the WGSL semantics on every GPU target: a
// workgroup control barrier, a load, and a second barrier, which is how Tint
// lowers the WGSL built-in for SPIR-V, HLSL and MSL (shader-slang/slang#13631).
// Thread 0 writes the groupshared value, every thread reads it, and thread 0
// then overwrites it, so the read must sit between the two barriers and the
// later write must come after the second one.

//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CUDA): -target cuda -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CPP): -target cpp -stage compute -entry computeMain

groupshared uint sharedVal;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID, uint gi: SV_GroupIndex)
{
    if (gi == 0)
        sharedVal = tid.x + 1;
    const uint n = workgroupUniformLoad(sharedVal);
    if (gi == 0)
        sharedVal = 0;
    outputBuffer[tid.x] = n;
}

// SPIRV: OpStore %sharedVal
// SPIRV: OpControlBarrier %uint_2 %uint_2 %uint_264
// SPIRV-NEXT: OpLoad %uint %sharedVal
// SPIRV-NEXT: OpControlBarrier %uint_2 %uint_2 %uint_264
// SPIRV-NOT: OpLoad %uint %sharedVal
// SPIRV: OpStore %sharedVal %uint_0

// HLSL: sharedVal{{.*}} = {{.*}}+ 1U;
// HLSL: GroupMemoryBarrierWithGroupSync()
// HLSL-NOT: GroupMemoryBarrierWithGroupSync
// HLSL: = sharedVal
// HLSL-NOT: sharedVal
// HLSL: GroupMemoryBarrierWithGroupSync()
// HLSL-NOT: = sharedVal
// HLSL: sharedVal{{.*}} = 0U;

// GLSL: sharedVal{{.*}} = {{.*}}+ 1U;
// GLSL: controlBarrier(gl_ScopeWorkgroup
// GLSL-NOT: controlBarrier
// GLSL: = sharedVal
// GLSL-NOT: sharedVal
// GLSL: controlBarrier(gl_ScopeWorkgroup
// GLSL-NOT: = sharedVal
// GLSL: sharedVal{{.*}} = 0U;

// METAL: sharedVal{{.*}} = {{.*}}+ 1U;
// METAL: threadgroup_barrier(mem_flags::mem_threadgroup)
// METAL-NOT: threadgroup_barrier
// METAL: = *{{.*}}sharedVal
// METAL-NOT: sharedVal
// METAL: threadgroup_barrier(mem_flags::mem_threadgroup)
// METAL-NOT: = *{{.*}}sharedVal
// METAL: sharedVal{{.*}} = 0U;

// CUDA: sharedVal{{.*}} = {{.*}}+ 1U;
// CUDA: __syncthreads()
// CUDA-NOT: __syncthreads
// CUDA: = *&sharedVal
// CUDA-NOT: sharedVal
// CUDA: __syncthreads()
// CUDA-NOT: = *&sharedVal
// CUDA: sharedVal{{.*}} = 0U;

// WGSL-NOT: workgroupBarrier
// WGSL: workgroupUniformLoad(&((sharedVal
// WGSL-NOT: workgroupUniformLoad(
// WGSL-NOT: workgroupBarrier

// The CPU target has no workgroup barrier, so it keeps the plain load.
// CPP-NOT: Barrier
// CPP: workgroupUniformLoad
// CPP-NOT: Barrier
