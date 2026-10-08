//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8 -experimental-feature
//TEST:SIMPLE(filecheck=CHK_HLSL): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

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
//CHK_HLSL: [NodeID("GeneratePath", 0)]
//CHK_HLSL: [NodeIsProgramEntry]
//CHK_HLSL: DispatchNodeInputRecord
