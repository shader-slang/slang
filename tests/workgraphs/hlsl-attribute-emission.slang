//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main -profile lib_6_8

// Test that work-graph attributes and native template types are preserved in emitted HLSL.

struct InputRecord
{
    uint3 grid : SV_DispatchGrid;
    int value;
};

struct OutputRecord
{
    int value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(4, 2, 1)]
[NodeID("MyNode")]
[NodeIsProgramEntry]
[numthreads(1, 1, 1)]
void main(
    DispatchNodeInputRecord<InputRecord> input,
    [MaxRecords(4)]
    [NodeID("TargetNode", 2)]
    [NodeArraySize(8)]
    [AllowSparseNodes]
    NodeOutputArray<OutputRecord> outputs)
{
    let inputRecord = input.Get();
    NodeOutput<OutputRecord> output = outputs[0];
    if (output.IsValid())
    {
        ThreadNodeOutputRecords<OutputRecord> records = output.GetThreadNodeOutputRecords(1);
        records[0].value = inputRecord.value;
        records.OutputComplete();
    }
}

//CHK: [shader("node")]
//CHK: [NodeLaunch("broadcasting")]
//CHK: [NodeMaxDispatchGrid(4, 2, 1)]
//CHK: [NodeID("MyNode", 0)]
//CHK: [NodeIsProgramEntry]
//CHK: DispatchNodeInputRecord<InputRecord
//CHK: [MaxRecords(4)]
//CHK: [NodeID("TargetNode", 2)]
//CHK: [NodeArraySize(8)]
//CHK: [AllowSparseNodes]
//CHK: NodeOutputArray<OutputRecord
//CHK-NOT: NodeOutputArray<NodeOutput<OutputRecord
