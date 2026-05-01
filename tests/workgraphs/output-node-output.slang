//TEST:SIMPLE(filecheck=CHK): -target dxil-asm -stage node -entry main -profile lib_6_8

// Test NodeOutput<T>, ThreadNodeOutputRecords<T>, IsValid(), and OutputComplete().

struct OutputRecord
{
    int value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main(RWBuffer<int> buf, [MaxRecords(4)] NodeOutput<OutputRecord> output)
{
    bool valid = output.IsValid();
    buf[0] = valid ? 1 : 0;
    ThreadNodeOutputRecords<OutputRecord> outRec = output.GetThreadNodeOutputRecords(1);
    outRec.OutputComplete();
}

//CHK: dx.op.nodeOutputIsValid
//CHK: dx.op.allocateNodeOutputRecords
//CHK: dx.op.outputComplete
