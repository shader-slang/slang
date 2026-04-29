//TEST:SIMPLE(filecheck=CHK): -target dxil-asm -stage node -entry main -profile lib_6_8

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
}

//CHK: dx.op.barrierByMemoryType(i32 244, i32 1, i32 1)
//CHK: dx.op.barrierByMemoryType(i32 244, i32 5, i32 6)
//CHK: dx.op.barrierByMemoryType(i32 244, i32 15, i32 1)
