//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main

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

//CHK: void main(
//CHK: [MaxRecords(4)]
//CHK: OutputComplete
