//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_GENERIC): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DGENERIC
import experimental.workgraph;

struct Record
{
    int value;
};

__generic<let N : int>
[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main([MaxRecords(N)] NodeOutput<Record> output)
//CHECK_GENERIC: could not extract value from integer constant
{
}
