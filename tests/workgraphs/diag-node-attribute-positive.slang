//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test that work-graph count/grid attributes reject non-positive values.

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
[NodeLaunch("thread")]
void recordsMain(
    [MaxRecords(0)] NodeOutput<OutputRecord> output,
//CHECK: E31406
    RWBuffer<int> buffer)
{
    buffer[0] = 0;
}
