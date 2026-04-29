//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8
//TEST:SIMPLE(filecheck=CHK_SPIRV): -target spirv-asm -stage node -entry main

// Test that SV_GroupIndex, SV_DispatchThreadID, SV_GroupID, SV_GroupThreadID
// are accepted as inputs in broadcasting-launch node shaders.

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(4, 1, 1)]
[numthreads(16, 1, 1)]
void main(
    uint groupIndex : SV_GroupIndex,
    uint3 dispatchThreadID : SV_DispatchThreadID,
    uint3 groupID : SV_GroupID,
    uint3 groupThreadID : SV_GroupThreadID,
    RWBuffer<uint> output)
{
    output[dispatchThreadID.x] = groupIndex + groupID.x + groupThreadID.x;
}

//CHK_DXIL: dx.op.flattenedThreadIdInGroup
//CHK_DXIL: dx.op.threadId
//CHK_SPIRV: OpFunction
