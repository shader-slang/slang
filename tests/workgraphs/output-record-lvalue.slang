//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8 -experimental-feature
//TEST:SIMPLE(filecheck=CHK_HLSL): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

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

//CHK_DXIL: dx.op.allocateNodeOutputRecords
//CHK_DXIL: dx.op.getNodeRecordPtr
//CHK_DXIL: dx.op.outputComplete
//CHK_HLSL: tRecs_0.Get(threadIndex_0).value_0 = int(42);
//CHK_HLSL: gRecs_0.Get(0U).value_0 = int(10);
//CHK_HLSL: gRecs_0.OutputComplete();
