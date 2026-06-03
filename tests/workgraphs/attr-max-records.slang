//TEST:SIMPLE(filecheck=CHK_DXIL): -target dxil-asm -stage node -entry main -profile lib_6_8
//TEST:SIMPLE(filecheck=CHK_HLSL): -target hlsl -stage node -entry main -profile lib_6_8

// Test that [MaxRecords(n)] is encoded in dxil.

struct MyRecord
{
    int value;
};

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main([MaxRecords(4)] NodeOutput<MyRecord> output, RWBuffer<int> buf)
{
    buf[0] = 0;
}

//CHK_DXIL: define void @main()
//CHK_HLSL: [MaxRecords(4)]
//CHK_HLSL: NodeOutput<MyRecord
