//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8
//TEST:SIMPLE(filecheck=CHK_SPIRV): -target spirv-asm -stage node -entry main -skip-spirv-validation

struct RecordData
{
    int myData;
};

RWStructuredBuffer<int> output;

[Shader("node")]
[NodeID("GeneratePath")]
[NodeIsProgramEntry]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(120, 270, 1)]
[NumThreads(16, 4, 1)]
void main(
    DispatchNodeInputRecord<RecordData> inputData,
    uint3 dispatchThreadId : SV_GroupThreadID)
{
    let recordData = inputData.Get();
    output[0] = recordData.myData;
}

//CHK_DXIL: dx.op.createNodeInputRecordHandle
//CHK_DXIL: dx.op.getNodeRecordPtr

//CHK_SPIRV: ; Types, variables and constants
//CHK_SPIRV: [[MemberType:%[a-zA-Z_0-9]+]] = OpTypeInt 32 1
//CHK_SPIRV: [[StructType:%[a-zA-Z_0-9]+]] = OpTypeStruct [[MemberType]]
//CHK_SPIRV: [[PayloadType:%[a-zA-Z_0-9]+]] = OpTypeNodePayloadArrayAMDX [[StructType]]
//CHK_SPIRV: [[PtrType:%[a-zA-Z_0-9]+]] = OpTypePointer NodePayloadAMDX [[PayloadType]]

//CHK_SPIRV: ; Function
//CHK_SPIRV: [[VarName:%[a-zA-Z_0-9]+]] = OpVariable [[PtrType]] NodePayloadAMDX
//CHK_SPIRV: OpAccessChain
//CHK_SPIRV: OpLoad
