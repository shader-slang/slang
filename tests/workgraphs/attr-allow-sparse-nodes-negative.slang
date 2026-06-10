//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NODE_OUTPUT,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DNODE_OUTPUT
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_EMPTY_OUTPUT,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DEMPTY_OUTPUT
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_DISPATCH_INPUT,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DDISPATCH_INPUT
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_GROUP_INPUT,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DGROUP_INPUT
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_THREAD_OUTPUT_RECORDS,non-exhaustive): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature -DTHREAD_OUTPUT_RECORDS
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
#elif defined(EMPTY_OUTPUT)
    [AllowSparseNodes]
//CHECK_EMPTY_OUTPUT: E31118
    EmptyNodeOutput output)
#elif defined(DISPATCH_INPUT)
    [AllowSparseNodes]
//CHECK_DISPATCH_INPUT: E31118
    DispatchNodeInputRecord<Record> input)
#elif defined(GROUP_INPUT)
    [AllowSparseNodes]
//CHECK_GROUP_INPUT: E31118
    GroupNodeInputRecords<Record> input)
#elif defined(THREAD_OUTPUT_RECORDS)
    [AllowSparseNodes]
//CHECK_THREAD_OUTPUT_RECORDS: E31118
    ThreadNodeOutputRecords<Record> output)
#endif
{
}
