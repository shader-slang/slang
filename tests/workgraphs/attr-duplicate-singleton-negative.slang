//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_MAX_RECORDS,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DMAX_RECORDS
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NODE_ID,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DNODE_ID
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NODE_IS_PROGRAM_ENTRY,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DNODE_IS_PROGRAM_ENTRY
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_ALLOW_SPARSE_NODES,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DALLOW_SPARSE_NODES
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NODE_ARRAY_SIZE,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DNODE_ARRAY_SIZE
import experimental.workgraph;

struct GridData
{
    uint3 grid : SV_DispatchGrid;
};

struct Record
{
    int value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
#if defined(NODE_ID)
[NodeID("A")]
[NodeID("B")]
//CHECK_NODE_ID: duplicate modifier
#elif defined(NODE_IS_PROGRAM_ENTRY)
[NodeIsProgramEntry]
[NodeIsProgramEntry]
//CHECK_NODE_IS_PROGRAM_ENTRY: duplicate modifier
#endif
[numthreads(1, 1, 1)]
void main(
    DispatchNodeInputRecord<GridData> input,
#if defined(MAX_RECORDS)
    [MaxRecords(1)] [MaxRecords(2)] NodeOutput<Record> output
    //CHECK_MAX_RECORDS: duplicate modifier
#elif defined(ALLOW_SPARSE_NODES)
    [MaxRecords(1)] [AllowSparseNodes] [AllowSparseNodes] NodeOutputArray<Record> output
    //CHECK_ALLOW_SPARSE_NODES: duplicate modifier
#elif defined(NODE_ARRAY_SIZE)
    [MaxRecords(1)] [NodeArraySize(1)] [NodeArraySize(2)] NodeOutputArray<Record> output
    //CHECK_NODE_ARRAY_SIZE: duplicate modifier
#else
    [MaxRecords(1)] NodeOutput<Record> output
#endif
)
{
}
