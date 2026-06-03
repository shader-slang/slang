//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test that work-graph count/grid attributes reject invalid ranges.

struct OutputRecord
{
    int value;
};

struct InputRecord
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
[NodeLaunch("thread")]
void recordsTooLargeMain(
    [MaxRecords(257)] NodeOutput<OutputRecord> output,
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
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NodeID("DefaultIndexNode")]
void nodeIDDefaultIndexMain(RWBuffer<int> output)
{
    output[0] = 0;
}

[shader("node")]
[NodeLaunch("thread")]
void nodeArrayTooLargeMain(
    [NodeArraySize(65536)] NodeOutputArray<OutputRecord> output,
//CHECK: E31406
    RWBuffer<int> buffer)
{
    buffer[0] = 0;
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

[shader("compute")]
[NodeLaunch("thread")]
//CHECK: E31408
[numthreads(1, 1, 1)]
void wrongStageNodeLaunchMain(RWBuffer<int> output)
{
    output[0] = 0;
}

[shader("compute")]
[NodeMaxDispatchGrid(1, 1, 1)]
//CHECK: E31408
[numthreads(1, 1, 1)]
void wrongStageNodeMaxDispatchGridMain(RWBuffer<int> output)
{
    output[0] = 0;
}

[shader("compute")]
[NodeDispatchGrid(1, 1, 1)]
//CHECK: E31408
[numthreads(1, 1, 1)]
void wrongStageNodeDispatchGridMain(RWBuffer<int> output)
{
    output[0] = 0;
}

[shader("compute")]
[NodeID("WrongStageNode")]
//CHECK: E31408
[numthreads(1, 1, 1)]
void wrongStageNodeIDMain(RWBuffer<int> output)
{
    output[0] = 0;
}

[shader("compute")]
[NodeIsProgramEntry]
//CHECK: E31408
[numthreads(1, 1, 1)]
void wrongStageNodeIsProgramEntryMain(RWBuffer<int> output)
{
    output[0] = 0;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void wrongStageParamMaxRecordsMain(
    [MaxRecords(1)] RWBuffer<int> output)
//CHECK: E31408
{
    output[0] = 0;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void wrongStageParamNodeIDMain(
    [NodeID("WrongStageParam")] RWBuffer<int> output)
//CHECK: E31408
{
    output[0] = 0;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void wrongStageParamNodeArraySizeMain(
    [NodeArraySize(1)] RWBuffer<int> output)
//CHECK: E31408
{
    output[0] = 0;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void wrongStageParamAllowSparseNodesMain(
    [AllowSparseNodes] RWBuffer<int> output)
//CHECK: E31408
{
    output[0] = 0;
}

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void dispatchInputMaxRecordsMain(
    [MaxRecords(4)] DispatchNodeInputRecord<InputRecord> input,
//CHECK: attribute 'MaxRecords' is not valid here
    RWBuffer<int> output)
{
    let value = input.Get().value;
    output[0] = value;
}

[shader("node")]
[NodeLaunch("thread")]
void threadInputMaxRecordsMain(
    [MaxRecords(4)] ThreadNodeInputRecord<InputRecord> input,
//CHECK: attribute 'MaxRecords' is not valid here
    RWBuffer<int> output)
{
    let value = input.Get().value;
    output[0] = value;
}
