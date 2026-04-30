//TEST:SIMPLE(filecheck=CHK): -target dxil-asm -stage node -entry main -profile lib_6_8

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

//CHK: dx.op.createNodeOutputHandle
//CHK: dx.op.allocateNodeOutputRecords
//CHK: dx.op.outputComplete
