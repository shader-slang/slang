//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// The subscript form also returns the selected element's address and carries an additional index
// argument through the BoundStorage lowering path.

struct Buffer
{
    int values[4];

    __subscript(int index) -> int
    {
        [mutating] ref { return values[index]; }
    }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Buffer buffer;
    for (int i = 0; i < 4; ++i)
        buffer.values[i] = 0;

    buffer[0] = 7;
    buffer[2] = 9;
    outputBuffer[0] = buffer.values[0];
    outputBuffer[1] = buffer.values[2];
    buffer[0] += 3;
    outputBuffer[2] = buffer.values[0];
    outputBuffer[3] = buffer.values[1];
}

// CHECK: 7
// CHECK-NEXT: 9
// CHECK-NEXT: 10
// CHECK-NEXT: 0
