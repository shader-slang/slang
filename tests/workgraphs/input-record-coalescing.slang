//TEST:SIMPLE(filecheck=CHK): -target dxil-asm -stage node -entry main -profile lib_6_8

// Test that GroupNodeInputRecords<T> compiles as a coalescing-launch node shader parameter.

struct RecordData
{
    int value;
};

[shader("node")]
[NodeLaunch("coalescing")]
[numthreads(1, 1, 1)]
void main(GroupNodeInputRecords<RecordData> input, RWBuffer<int> output)
{
    output[0] = input.Get(0).value;
}

//CHK: dx.op.createNodeInputRecordHandle
//CHK: dx.op.getNodeRecordPtr
