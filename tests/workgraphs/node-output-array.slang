//TEST:SIMPLE(filecheck=CHK): -target dxil-asm -stage node -entry main -profile lib_6_8

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

//CHK: dx.op.createNodeOutputHandle
//CHK: dx.op.indexNodeHandle
//CHK: dx.op.allocateNodeOutputRecords
