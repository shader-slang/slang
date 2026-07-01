// GroupMemoryBarrier (no sync) emission across HLSL, GLSL, and SPIRV.
//
// From the docs: "Related barriers include GroupMemoryBarrier() and
// GroupMemoryBarrierWithGroupSync()." The "no-sync" variant orders
// memory operations within each thread without synchronizing the
// thread group, so the SPIRV emission must use OpMemoryBarrier
// (without OpControlBarrier).

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv-asm -stage compute -entry computeMain -emit-spirv-directly

RWStructuredBuffer<int> outputBuffer;
groupshared int sharedData[4];

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, uint idx : SV_GroupIndex)
{
    sharedData[idx] = int(idx);

    // Memory-only fence — no group synchronization. Distinct from
    // the WithGroupSync variant exercised in groupshared-barrier-*.
    GroupMemoryBarrier();

    outputBuffer[idx] = sharedData[idx];
}

// HLSL keeps the explicit name.
// CHECK_HLSL: GroupMemoryBarrier

// GLSL emits a memory barrier scoped to the workgroup with shared
// storage semantics. Slang-emitted GLSL uses the explicit
// memoryBarrier(scope, storage, semantics) form.
// CHECK_GLSL-DAG: memoryBarrier
// CHECK_GLSL-DAG: gl_StorageSemanticsShared

// SPIRV emits OpMemoryBarrier without an accompanying OpControlBarrier.
// CHECK_SPIRV: OpMemoryBarrier
// CHECK_SPIRV-NOT: OpControlBarrier
