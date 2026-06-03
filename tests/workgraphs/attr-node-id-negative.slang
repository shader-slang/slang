//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

struct Record
{
    int value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main([MaxRecords(1)] [NodeID("TargetNode", -1)] NodeOutput<Record> output)
//CHECK: NodeID arrayIndex must be in the range 0 to 0xffffffff, got -1
{
}
