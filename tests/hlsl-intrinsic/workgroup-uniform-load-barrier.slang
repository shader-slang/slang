// In a compute shader, `workgroupUniformLoad` returns every thread the value of
// `v` at a workgroup barrier, unaffected by a later write
// (shader-slang/slang#13631). Off WGSL it is lowered as barrier, load, barrier.
// Thread 0 writes the groupshared value, every thread reads it, and thread 0
// then overwrites it. Each GPU lane checks the write, the first barrier,
// exactly one read of `sharedVal`, the second barrier, no further read, and
// then the later write.

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

// On Metal, the emit-time load deferral can move the load past the second
// barrier when nothing between them writes `sharedVal`
// (shader-slang/slang#13409). The write after the call keeps the load in place.
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

// The CPU targets have no workgroup barrier, so the C++ helper is a plain load.
// CPP: workgroupUniformLoad_0(uint32_t * v_0)
// CPP-NOT: Barrier
// CPP: return *v_0;
// CPP-NOT: Barrier
