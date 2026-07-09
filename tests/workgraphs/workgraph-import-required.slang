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
//CHECK: unknown attribute 'NodeDispatchGrid'
[NodeMaxDispatchGrid(1, 1, 1)]
//CHECK: unknown attribute 'NodeMaxDispatchGrid'
[NodeID("MissingImport")]
//CHECK: unknown attribute 'NodeID'
[NodeIsProgramEntry]
//CHECK: unknown attribute 'NodeIsProgramEntry'
[numthreads(1, 1, 1)]
void main(
    DispatchNodeInputRecord<Record> dispatchInput,
    //CHECK: undefined identifier 'DispatchNodeInputRecord'
    ThreadNodeInputRecord<Record> threadInput,
    //CHECK: undefined identifier 'ThreadNodeInputRecord'
    GroupNodeInputRecords<Record> groupInput,
    //CHECK: undefined identifier 'GroupNodeInputRecords'
    EmptyNodeInput emptyInput,
    //CHECK: undefined identifier 'EmptyNodeInput'
    [MaxRecords(1)] NodeOutput<Record> output,
//CHECK: unknown attribute 'MaxRecords'
//CHECK: undefined identifier 'NodeOutput'
    ThreadNodeOutputRecords<Record> threadOutput,
    //CHECK: undefined identifier 'ThreadNodeOutputRecords'
    GroupNodeOutputRecords<Record> groupOutput,
    //CHECK: undefined identifier 'GroupNodeOutputRecords'
    [NodeArraySize(2)] [AllowSparseNodes] NodeOutputArray<Record> outputArray,
    //CHECK: unknown attribute 'NodeArraySize'
    //CHECK: unknown attribute 'AllowSparseNodes'
    //CHECK: undefined identifier 'NodeOutputArray'
    EmptyNodeOutput emptyOutput,
    //CHECK: undefined identifier 'EmptyNodeOutput'
    EmptyNodeOutputArray emptyOutputArray)
//CHECK: undefined identifier 'EmptyNodeOutputArray'
{
    Barrier(BarrierMemoryTypeFlags.UAV_MEMORY, BarrierSemanticFlags.GROUP_SYNC);
    //CHECK: undefined identifier 'Barrier'
    //CHECK: undefined identifier 'BarrierMemoryTypeFlags'
    //CHECK: undefined identifier 'BarrierSemanticFlags'
}
