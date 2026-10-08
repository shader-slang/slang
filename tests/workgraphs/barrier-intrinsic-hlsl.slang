//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

// Test that Barrier() emits HLSL's named flag constants for single, combined, and zero-valued flags.

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main()
{
    Barrier(BarrierMemoryTypeFlags.ALL_MEMORY, BarrierSemanticFlags.REORDER);
    Barrier(BarrierMemoryTypeFlags.UAV_MEMORY, (BarrierSemanticFlags)0);
    Barrier(BarrierMemoryTypeFlags.UAV_MEMORY, BarrierSemanticFlags.GROUP_SYNC);
    Barrier(BarrierMemoryTypeFlags.GROUP_SHARED_MEMORY, BarrierSemanticFlags.GROUP_SYNC);
    Barrier(
        BarrierMemoryTypeFlags.UAV_MEMORY |
            BarrierMemoryTypeFlags.NODE_INPUT_MEMORY |
            BarrierMemoryTypeFlags.NODE_OUTPUT_MEMORY,
        BarrierSemanticFlags.GROUP_SCOPE | BarrierSemanticFlags.DEVICE_SCOPE);
    Barrier(
        BarrierMemoryTypeFlags.GROUP_SHARED_MEMORY |
            BarrierMemoryTypeFlags.NODE_INPUT_MEMORY |
            BarrierMemoryTypeFlags.NODE_OUTPUT_MEMORY,
        BarrierSemanticFlags.GROUP_SYNC);
}

//CHK: Barrier((ALL_MEMORY), (REORDER));
//CHK: Barrier((UAV_MEMORY), (REORDER));
//CHK: Barrier((UAV_MEMORY), (GROUP_SYNC));
//CHK: Barrier((GROUP_SHARED_MEMORY), (GROUP_SYNC));
//CHK: Barrier((UAV_MEMORY | NODE_INPUT_MEMORY | NODE_OUTPUT_MEMORY), (GROUP_SCOPE | DEVICE_SCOPE));
//CHK: Barrier((GROUP_SHARED_MEMORY | NODE_INPUT_MEMORY | NODE_OUTPUT_MEMORY), (GROUP_SYNC));
