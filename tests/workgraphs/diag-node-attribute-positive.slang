//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test that work-graph count/grid attributes reject invalid ranges.

struct OutputRecord
{
    int value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(0, 1, 1)]
//CHECK: E31406
void gridMain(RWBuffer<int> output)
{
    output[0] = 0;
}

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(70000, 1, 1)]
//CHECK: E31406
void gridTooLargeMain(RWBuffer<int> output)
{
    output[0] = 0;
}

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(1, 65536, 1)]
//CHECK: E31406
void maxGridTooLargeMain(RWBuffer<int> output)
{
    output[0] = 0;
}

[shader("node")]
[NodeLaunch("thread")]
void recordsMain(
    [MaxRecords(0)] NodeOutput<OutputRecord> output,
//CHECK: E31406
    RWBuffer<int> buffer)
{
    buffer[0] = 0;
}

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NodeID("BadNode", -1)]
//CHECK: E31406
void nodeIDMain(RWBuffer<int> output)
{
    output[0] = 0;
}

[shader("node")]
[NodeLaunch("coalescing")]
[MaxRecords(8)]
//CHECK: attribute 'MaxRecords' is not valid here
[numthreads(1, 1, 1)]
void functionMaxRecordsMain(RWBuffer<int> output)
{
    output[0] = 0;
}
