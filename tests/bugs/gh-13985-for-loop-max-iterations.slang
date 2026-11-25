//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[3 0 0], stride=4):out,name=outBuf
RWStructuredBuffer<uint> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint n = 0;
    for (uint64_t i = 0; i < 0xFFFFFFFFFFFFFFFFull; i++)
    {
        n++;
        if (n == outBuf[0])
            break;
    }
    outBuf[1] = n;

    uint m = 0;
    for (uint64_t j = 5; j < 0x8000000000000000ull; j++)
    {
        m++;
        if (m == outBuf[0])
            break;
    }
    outBuf[2] = m;
}
// CHECK: 3
// CHECK-NEXT: 3
// CHECK-NEXT: 3
