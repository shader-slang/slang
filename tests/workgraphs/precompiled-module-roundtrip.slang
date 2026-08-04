//TEST:SIMPLE: -o tests/workgraphs/precompiled-module-roundtrip.slang-module -experimental-feature
//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry dispatchMain -profile lib_6_8 -experimental-feature tests/workgraphs/precompiled-module-roundtrip.slang-module
import experimental.workgraph;

struct Record
{
    uint value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NodeID("DispatchNode")]
[NodeIsProgramEntry]
[numthreads(4, 1, 1)]
void dispatchMain(
    DispatchNodeInputRecord<Record> input,
    [MaxRecords(1)] NodeOutput<Record> output,
    [NodeID("ArrayTarget", 1)]
    [NodeArraySize(2)]
    [AllowSparseNodes]
    EmptyNodeOutputArray emptyOutputs)
{
    GroupNodeOutputRecords<Record> records = output.GetGroupNodeOutputRecords(1);
    records.Get().value = input.Get().value;
    records.OutputComplete();
    if (emptyOutputs[0].IsValid())
        emptyOutputs[0].ThreadIncrementOutputCount(1);
    Barrier(
        BarrierMemoryTypeFlags.UAV_MEMORY | BarrierMemoryTypeFlags.NODE_OUTPUT_MEMORY,
        BarrierSemanticFlags.GROUP_SYNC | BarrierSemanticFlags.GROUP_SCOPE);
}

[shader("node")]
[NodeLaunch("coalescing")]
[MaxRecords(4)]
[numthreads(4, 1, 1)]
void coalescingMain(GroupNodeInputRecords<Record> input, EmptyNodeInput emptyInput)
{
    uint count = input.Count() + emptyInput.Count();
}

[shader("node")]
[NodeLaunch("thread")]
void threadMain(
    ThreadNodeInputRecord<Record> input,
    [MaxRecords(1)] NodeOutput<Record> output,
    EmptyNodeOutput emptyOutput)
{
    ThreadNodeOutputRecords<Record> records = output.GetThreadNodeOutputRecords(1);
    records[0].value = input.Get().value;
    records.OutputComplete();
    if (emptyOutput.IsValid())
        emptyOutput.ThreadIncrementOutputCount(1);
}

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void maxGridMain([MaxRecords(1)] NodeOutputArray<Record> outputs)
{
}

//CHK: [NodeLaunch("broadcasting")]
//CHK: [NodeID("DispatchNode", 0)]
//CHK: [NodeIsProgramEntry]
//CHK: DispatchNodeInputRecord
//CHK: [MaxRecords(1)] NodeOutput
//CHK: [NodeID("ArrayTarget", 1)] [NodeArraySize(2)] [AllowSparseNodes] EmptyNodeOutputArray
//CHK: Barrier((UAV_MEMORY | NODE_OUTPUT_MEMORY), (GROUP_SYNC | GROUP_SCOPE));
