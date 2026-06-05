//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NODE_OUTPUT,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DNODE_OUTPUT
import experimental.workgraph;

struct Record
{
    int value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main(
#if defined(NODE_OUTPUT)
    [AllowSparseNodes]
//CHECK_NODE_OUTPUT: E31118
    NodeOutput<Record> output)
#endif
{
}
