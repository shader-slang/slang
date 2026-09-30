//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type

//TEST_INPUT:ubuffer(data=[10 20 30], stride=4):name=storage
RWStructuredBuffer<uint> storage;

// Bind the default 56-byte view and the 72-byte wide view to the same storage.
//TEST_INPUT:ubuffer(data=[storage 4 0 0 0 0 3 0 0 0 0 1 0], stride=56):name=narrow
uniform TensorView<uint>* narrow;
//TEST_INPUT:ubuffer(data=[storage 4 0 0 0 0 0 0 0 0 0 3 0 0 0 0 1], stride=72):name=wide
uniform TensorView<uint, uint64_t>* wide;

//TEST_INPUT:ubuffer(count=5, stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = storage[0];
    outputBuffer[1] = narrow.load(1);
    wide.store(1, 40);
    outputBuffer[2] = narrow.load(1);
    uint oldValue;
    wide.InterlockedAdd(1, 2, oldValue);
    outputBuffer[3] = oldValue;
    wide.InterlockedMin(1, 41, oldValue);
    outputBuffer[4] = wide.load(1);
}

// CHECK: 10
// CHECK: 20
// CHECK: 40
// CHECK: 40
// CHECK: 41
