//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8 -experimental-feature
//TEST:SIMPLE(filecheck=CHK_HLSL): -target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

// Test that NodeOutputArray compiles and emits indexing correctly.

struct MyRecord
{
    uint value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(4, 1, 1)]
[numthreads(4, 1, 1)]
void main(
    DispatchNodeInputRecord<MyRecord> input,
    [MaxRecords(4)] NodeOutputArray<MyRecord> outputs)
{
    NodeOutput<MyRecord> out = outputs[0];
    if (out.IsValid())
    {
        ThreadNodeOutputRecords<MyRecord> rec = out.GetThreadNodeOutputRecords(1);
        rec.Get().value = input.Get().value;
        rec.OutputComplete();
    }
}

//CHK_DXIL: dx.op.createNodeOutputHandle
//CHK_DXIL: dx.op.indexNodeHandle
//CHK_DXIL: dx.op.allocateNodeOutputRecords
//CHK_HLSL: [MaxRecords(4)] NodeOutputArray
//CHK_HLSL: NodeOutputArray_operator
//CHK_HLSL: outputs_0, 0U
//CHK_HLSL: ThreadNodeOutputRecords
