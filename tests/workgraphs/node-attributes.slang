//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8
//TEST:SIMPLE(filecheck=CHK_HLSL): -target hlsl -stage node -entry main -profile lib_6_8

// Test that NodeID, NodeIsProgramEntry, NodeArraySize, AllowSparseNodes compile to dxil.

struct OutputRecord { int value; };

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
[NodeID("MyNode", 2)]
[NodeIsProgramEntry]
void main(
    [MaxRecords(4)]
    [NodeArraySize(8)]
    [AllowSparseNodes]
    NodeOutputArray<OutputRecord> outputs,
    [NodeID("TargetNode", 0)]
    NodeOutput<OutputRecord> single)
{
}

//CHK_DXIL: !"MyNode", i32 2
//CHK_DXIL: !"TargetNode", i32 0
//CHK_HLSL: [NodeID("MyNode", 2)]
//CHK_HLSL: [NodeIsProgramEntry]
//CHK_HLSL: [MaxRecords(4)]
//CHK_HLSL: [NodeArraySize(8)]
//CHK_HLSL: [AllowSparseNodes]
//CHK_HLSL: NodeOutputArray<OutputRecord
//CHK_HLSL: [NodeID("TargetNode", 0)]
