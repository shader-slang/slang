//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -output-using-type

//TEST_INPUT:ubuffer(data=[10 20 30], stride=4):name=storage
RWStructuredBuffer<uint> storage;

// Bind the default 56-byte view and the 96-byte wide view to the same storage.
//TEST_INPUT:ubuffer(data=[storage 4 0 0 0 0 3 0 0 0 0 1 0], stride=56):name=narrow
uniform TensorView<uint>* narrow;
//TEST_INPUT:ubuffer(data=[storage 4 0 4 0 4 0 4 0 4 0 3 0 1 0 1 0 1 0 1 0 5 0], stride=96):name=wide
uniform TensorView<uint, uint64_t>* wide;

//TEST_INPUT:ubuffer(count=11, stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = storage[0];
    outputBuffer[1] = narrow.load(1);
    wide.store(1, 0, 0, 0, 0, 40);
    outputBuffer[2] = narrow.load(1);
    uint oldValue;
    wide.InterlockedAdd(vector<uint64_t, 2>(1, 0), 2, oldValue);
    outputBuffer[3] = oldValue;
    wide.InterlockedMin(vector<uint64_t, 4>(1, 0, 0, 0), 41, oldValue);
    outputBuffer[4] = (*wide)[vector<uint64_t, 3>(1, 0, 0)];

    wide.store(vector<uint64_t, 2>(1, 0), 50);
    outputBuffer[5] = wide.load(1, 0);
    wide.store(1, 0, 0, 60);
    outputBuffer[6] = wide.load(vector<uint64_t, 3>(1, 0, 0));
    wide.store(vector<uint64_t, 4>(1, 0, 0, 0), 70);
    outputBuffer[7] = wide.load(1, 0, 0, 0);
    (*wide)[1, 0, 0, 0, 0] = 80;
    outputBuffer[8] = wide.load(1, 0, 0, 0, 0);
    outputBuffer[9] = narrow.load(uint2(1, 0));
    outputBuffer[10] = *(&(*narrow)[1]);
}

// CHECK: 10
// CHECK: 20
// CHECK: 40
// CHECK: 40
// CHECK: 41
// CHECK: 50
// CHECK: 60
// CHECK: 70
// CHECK: 80
// CHECK: 80
// CHECK: 80
