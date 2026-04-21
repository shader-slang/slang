//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main

// Test that output record subscript returns an l-value so fields can be written,
// and that GetGroupNodeOutputRecords is available on NodeOutput<T>.

struct OutData
{
    int value;
    int index;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(4, 1, 1)]
void main(
    uint threadIndex : SV_GroupIndex,
    [MaxRecords(4)] NodeOutput<OutData> threadOut,
    [MaxRecords(2)] NodeOutput<OutData> groupOut)
{
    ThreadNodeOutputRecords<OutData> tRecs = threadOut.GetThreadNodeOutputRecords(1);
    tRecs[threadIndex].value = 42;
    tRecs[threadIndex].index = (int)threadIndex;
    tRecs.OutputComplete();

    GroupNodeOutputRecords<OutData> gRecs = groupOut.GetGroupNodeOutputRecords(2);
    gRecs[0].value = 10;
    gRecs.OutputComplete();
}

//CHK: void main(
//CHK: GetThreadNodeOutputRecords
//CHK: .Get(
//CHK: GetGroupNodeOutputRecords
//CHK: OutputComplete
