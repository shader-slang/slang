//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8 -experimental-feature
//TEST:SIMPLE(filecheck=CHK_HLSL): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

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
    Barrier(BarrierMemoryTypeFlags.UAV_MEMORY | BarrierMemoryTypeFlags.NODE_INPUT_MEMORY | BarrierMemoryTypeFlags.NODE_OUTPUT_MEMORY, BarrierSemanticFlags.GROUP_SCOPE | BarrierSemanticFlags.DEVICE_SCOPE);
    // ALL_MEMORY convenience constant
    Barrier(BarrierMemoryTypeFlags.ALL_MEMORY, BarrierSemanticFlags.GROUP_SYNC);
}

//CHK_DXIL: dx.op.barrierByMemoryType(i32 244, i32 1, i32 1)
//CHK_DXIL: dx.op.barrierByMemoryType(i32 244, i32 13, i32 6)
//CHK_DXIL: dx.op.barrierByMemoryType(i32 244, i32 15, i32 1)
//CHK_HLSL: Barrier((UAV_MEMORY), (GROUP_SYNC));
//CHK_HLSL: Barrier((UAV_MEMORY | NODE_INPUT_MEMORY | NODE_OUTPUT_MEMORY), (GROUP_SCOPE | DEVICE_SCOPE));
//CHK_HLSL: Barrier((ALL_MEMORY), (GROUP_SYNC));
