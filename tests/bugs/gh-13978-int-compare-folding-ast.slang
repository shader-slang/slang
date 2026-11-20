//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

static const uint64_t X = 0xFFFFFFFFFFFFFFFFull;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint64_t x = 0xFFFFFFFFFFFFFFFFull;
    int arr[(X > 5ull) ? 1 : 2];
    outputBuffer[0] = arr.getCount();                         // front-end fold
}
// CHECK: type: int32_t
// CHECK-NEXT: 1
