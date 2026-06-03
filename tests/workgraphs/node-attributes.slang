//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

// Test that NodeID, NodeIsProgramEntry, NodeArraySize, AllowSparseNodes compile to dxil.

struct OutputRecord { int value; };

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
[NodeID("MyNode")]
[NodeIsProgramEntry]
void main(
    [MaxRecords(4)]
    [NodeArraySize(8)]
    [AllowSparseNodes]
    NodeOutputArray<OutputRecord> outputs,
    [NodeID("TargetNode", 0)]
    NodeOutput<OutputRecord> single,
    [NodeID("MaxNode", 0xffffffff)]
    NodeOutput<OutputRecord> maxIndex)
{
}

//CHK_DXIL: !"MyNode", i32 0
//CHK_DXIL: !"TargetNode", i32 0
//CHK_DXIL: !"MaxNode", i32 {{-1|4294967295}}
