//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_LAUNCH_EXTRA,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DLAUNCH_EXTRA
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_MAX_GRID_MISSING,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DMAX_GRID_MISSING
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_FIXED_GRID_EXTRA,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DFIXED_GRID_EXTRA
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_MAX_RECORDS_MISSING,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DMAX_RECORDS_MISSING
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NODE_ID_MISSING,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DNODE_ID_MISSING
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_ARRAY_SIZE_EXTRA,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DARRAY_SIZE_EXTRA
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_PROGRAM_ENTRY_EXTRA,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DPROGRAM_ENTRY_EXTRA
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_ALLOW_SPARSE_EXTRA,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DALLOW_SPARSE_EXTRA
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
#if defined(LAUNCH_EXTRA)
[NodeLaunch("broadcasting", "extra")]
//CHECK_LAUNCH_EXTRA: attribute 'NodeLaunch' expects 1 arguments (2 provided)
[NodeDispatchGrid(1, 1, 1)]
#elif defined(MAX_GRID_MISSING)
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(1, 1)]
//CHECK_MAX_GRID_MISSING: attribute 'NodeMaxDispatchGrid' expects 3 arguments (2 provided)
#elif defined(FIXED_GRID_EXTRA)
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1, 1)]
//CHECK_FIXED_GRID_EXTRA: attribute 'NodeDispatchGrid' expects 3 arguments (4 provided)
#else
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
#endif
#if defined(PROGRAM_ENTRY_EXTRA)
[NodeIsProgramEntry(1)]
//CHECK_PROGRAM_ENTRY_EXTRA: attribute 'NodeIsProgramEntry' expects 0 arguments (1 provided)
#endif
[numthreads(1, 1, 1)]
void main(
    DispatchNodeInputRecord<GridData> input,
#if defined(MAX_RECORDS_MISSING)
    [MaxRecords()] NodeOutput<Record> output
    //CHECK_MAX_RECORDS_MISSING: attribute 'MaxRecords' expects 1 arguments (0 provided)
#elif defined(NODE_ID_MISSING)
    [MaxRecords(1)] [NodeID()] NodeOutput<Record> output
    //CHECK_NODE_ID_MISSING: attribute 'NodeID' expects 2 arguments (0 provided)
#elif defined(ARRAY_SIZE_EXTRA)
    [MaxRecords(1)] [NodeArraySize(1, 2)] NodeOutputArray<Record> output
    //CHECK_ARRAY_SIZE_EXTRA: attribute 'NodeArraySize' expects 1 arguments (2 provided)
#elif defined(ALLOW_SPARSE_EXTRA)
    [MaxRecords(1)] [NodeArraySize(1)] [AllowSparseNodes(1)] NodeOutputArray<Record> output
    //CHECK_ALLOW_SPARSE_EXTRA: attribute 'AllowSparseNodes' expects 0 arguments (1 provided)
#else
    [MaxRecords(1)] NodeOutput<Record> output
#endif
)
{
}
