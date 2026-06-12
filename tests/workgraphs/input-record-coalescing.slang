//TEST:SIMPLE(filecheck=CHK): -target dxil-asm -stage node -entry main -profile lib_6_8
//TEST:SIMPLE(filecheck=CHK_HLSL): -target hlsl -stage node -entry main -profile lib_6_8

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
    uint sum = 0;
    for (uint i = 0; i < input.Count(); ++i)
    {
        sum += input[i].value;
    }
    output[0] = sum;
}

//CHK: dx.op.createNodeInputRecordHandle
//CHK: dx.op.getInputRecordCount
//CHK: dx.op.getNodeRecordPtr
//CHK_HLSL: [NodeLaunch("coalescing")]
//CHK_HLSL: GroupNodeInputRecords<RecordData
//CHK_HLSL: .Count()
//CHK_HLSL: GroupNodeInputRecords_operatorx5Bx5D_get
