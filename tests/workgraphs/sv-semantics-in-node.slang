//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8 -experimental-feature
//TEST:SIMPLE(filecheck=CHK_SPIRV): -target spirv-asm -stage node -entry main -experimental-feature
import experimental.workgraph;

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
//CHK_SPIRV: OpDecorate [[GLOBAL_ID:%[a-zA-Z_0-9]+]] BuiltIn GlobalInvocationId
//CHK_SPIRV: OpDecorate [[GROUP_ID:%[a-zA-Z_0-9]+]] BuiltIn WorkgroupId
//CHK_SPIRV: OpDecorate [[GROUP_INDEX:%[a-zA-Z_0-9]+]] BuiltIn LocalInvocationIndex
//CHK_SPIRV: OpDecorate [[GROUP_THREAD_ID:%[a-zA-Z_0-9]+]] BuiltIn LocalInvocationId
//CHK_SPIRV: OpFunction
//CHK_SPIRV: OpLoad %v3uint [[GLOBAL_ID]]
//CHK_SPIRV: OpLoad %v3uint [[GROUP_ID]]
//CHK_SPIRV: OpLoad %uint [[GROUP_INDEX]]
//CHK_SPIRV: OpLoad %v3uint [[GROUP_THREAD_ID]]
