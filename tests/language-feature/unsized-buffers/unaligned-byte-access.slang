//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -llvm -compute -output-using-type

// Byte-address loads and stores require only four-byte alignment for uint64_t. Converting to
// an unsized view must preserve this contract when it omits the byte count.
//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4):name=inputBytes
ByteAddressBuffer inputBytes;
//TEST_INPUT:ubuffer(data=[111 0 0 444], stride=4):name=outputBytes
RWByteAddressBuffer outputBytes;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint64_t sizedValue = inputBytes.Load<uint64_t>(4);
    __UnsizedByteAddressBuffer inputView = inputBytes;
    uint64_t unsizedValue = inputView.Load<uint64_t>(4);
    output[0] = uint(sizedValue);
    output[1] = uint(sizedValue >> 32);
    output[2] = uint(unsizedValue);
    output[3] = uint(unsizedValue >> 32);

    __UnsizedRWByteAddressBuffer outputView = outputBytes;
    outputView.Store<uint64_t>(4, sizedValue);
    output[4] = outputBytes.Load(4);
    output[5] = outputBytes.Load(8);
    output[6] = outputBytes.Load(0);
    output[7] = outputBytes.Load(12);
}

// CHECK: 2
// CHECK: 3
// CHECK: 2
// CHECK: 3
// CHECK: 2
// CHECK: 3
// CHECK: 111
// CHECK: 444
