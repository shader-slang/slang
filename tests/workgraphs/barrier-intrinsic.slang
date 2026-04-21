//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main

// Test that Barrier() emits HLSL named constants, not raw integers.

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

//CHK: Barrier((UAV_MEMORY), (GROUP_SYNC))
//CHK: Barrier((UAV_MEMORY | NODE_INPUT_MEMORY), (GROUP_SCOPE | DEVICE_SCOPE))
//CHK: Barrier((ALL_MEMORY), (GROUP_SYNC))
