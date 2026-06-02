//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature

// Work graph APIs must be imported from the experimental workgraph module.

struct Record
{
    int value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
//CHECK: unknown attribute 'NodeLaunch'
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main([MaxRecords(1)] NodeOutput<Record> output)
//CHECK: undefined identifier 'NodeOutput'
{
}
