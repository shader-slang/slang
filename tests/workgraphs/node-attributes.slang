//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main

// Test that NodeID, NodeIsProgramEntry, NodeArraySize, AllowSparseNodes emit correctly.

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

//CHK: [NodeID("MyNode", 2)]
//CHK: [NodeIsProgramEntry]
//CHK: [MaxRecords(4)]
//CHK: [NodeArraySize(8)]
//CHK: [AllowSparseNodes]
//CHK: [NodeID("TargetNode", 0)]
