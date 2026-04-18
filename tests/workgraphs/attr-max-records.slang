//TEST:SIMPLE(filecheck=CHK): -target hlsl -stage node -entry main

// Test that [MaxRecords(n)] is emitted on a NodeOutput parameter.

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

//CHK: [MaxRecords(4)]
