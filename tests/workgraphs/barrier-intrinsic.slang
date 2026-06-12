//TEST:SIMPLE(filecheck=CHK): -target dxil-asm -stage node -entry main -profile lib_6_8
//TEST:SIMPLE(filecheck=CHK_HLSL): -target hlsl -stage node -entry main -profile lib_6_8

// Test that Barrier() emits the correct DXIL intrinsic with proper flag values.

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main(RWBuffer<int> buf)
{
    buf[0] = 1;
    // Single flags
    Barrier(BarrierMemoryTypeFlags.UAV_MEMORY, BarrierSemanticFlags.GROUP_SYNC);
    // Combined flags
    Barrier(BarrierMemoryTypeFlags.UAV_MEMORY | BarrierMemoryTypeFlags.NODE_INPUT_MEMORY, BarrierSemanticFlags.GROUP_SCOPE | BarrierSemanticFlags.DEVICE_SCOPE);
    // ALL_MEMORY convenience constant
    Barrier(BarrierMemoryTypeFlags.ALL_MEMORY, BarrierSemanticFlags.GROUP_SYNC);
    // Remaining named memory constants and a no-op semantic flag set.
    Barrier(BarrierMemoryTypeFlags.GROUP_SHARED_MEMORY, BarrierSemanticFlags.GROUP_SYNC);
    Barrier(BarrierMemoryTypeFlags.NODE_OUTPUT_MEMORY, BarrierSemanticFlags.GROUP_SYNC);
    Barrier(BarrierMemoryTypeFlags.UAV_MEMORY, (BarrierSemanticFlags)0);
    Barrier((BarrierMemoryTypeFlags)0, BarrierSemanticFlags.GROUP_SYNC);
}

//CHK: dx.op.barrierByMemoryType(i32 244, i32 1, i32 1)
//CHK: dx.op.barrierByMemoryType(i32 244, i32 5, i32 6)
//CHK: dx.op.barrierByMemoryType(i32 244, i32 15, i32 1)
//CHK: dx.op.barrierByMemoryType(i32 244, i32 2, i32 1)
//CHK: dx.op.barrierByMemoryType(i32 244, i32 8, i32 1)
//CHK: dx.op.barrierByMemoryType(i32 244, i32 1, i32 0)
//CHK: dx.op.barrierByMemoryType(i32 244, i32 0, i32 1)

//CHK_HLSL: Barrier((UAV_MEMORY), (GROUP_SYNC))
//CHK_HLSL: Barrier((UAV_MEMORY | NODE_INPUT_MEMORY), (GROUP_SCOPE | DEVICE_SCOPE))
//CHK_HLSL: Barrier((ALL_MEMORY), (GROUP_SYNC))
//CHK_HLSL: Barrier((GROUP_SHARED_MEMORY), (GROUP_SYNC))
//CHK_HLSL: Barrier((NODE_OUTPUT_MEMORY), (GROUP_SYNC))
//CHK_HLSL: Barrier((UAV_MEMORY), (0))
//CHK_HLSL: Barrier((0), (GROUP_SYNC))
