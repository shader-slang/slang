//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

static const uint64_t X = 0xFFFFFFFFFFFFFFFFull;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint64_t x = 0xFFFFFFFFFFFFFFFFull;
    outputBuffer[0] = (x > 5ull) ? 1 : 2;                     // IR SCCP
    outputBuffer[1] = (0x8000000000000000ull > 1ull) ? 1 : 2; // IR SCCP
    outputBuffer[2] = (x >= 5ull) ? 1 : 2;                    // IR SCCP
}
// CHECK: type: int32_t
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
