//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8 -experimental-feature
//TEST:SIMPLE(filecheck=CHK_HLSL): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

// Test that GroupNodeOutputRecords compiles and emits correctly.

struct MyRecord
{
    uint value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(4, 1, 1)]
void main(
    DispatchNodeInputRecord<MyRecord> input,
    [MaxRecords(1)] NodeOutput<MyRecord> output)
{
    GroupNodeOutputRecords<MyRecord> rec = output.GetGroupNodeOutputRecords(1);
    rec.Get().value = input.Get().value;
    rec.OutputComplete();
}

//CHK_DXIL: dx.op.createNodeOutputHandle
//CHK_DXIL: dx.op.allocateNodeOutputRecords
//CHK_DXIL: dx.op.outputComplete
//CHK_HLSL: [MaxRecords(1)] NodeOutput
//CHK_HLSL: GroupNodeOutputRecords
//CHK_HLSL: rec_0.OutputComplete();
