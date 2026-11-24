//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    int arr0[(0u - 1u) / 2u ==  0x7FFFFFFFU ? 1 : 0];
    int arr1[(2147483647 + 1) / 2 == -1073741824 ? 2 : 0];
    int arr2[((0u - 1u) >> 1u) == 2147483647 ? 3 : 0];
    int arr3[(0x8000000000000000ull / 0xFFFFFFFFFFFFFFFFull == 0ull) ? 4 : 0];

    output[0] = arr0.getCount();
    output[1] = arr1.getCount();
    output[2] = arr2.getCount();
    output[3] = arr3.getCount();
}

// BUF:      type: int32_t
// BUF-NEXT: 1
// BUF-NEXT: 2
// BUF-NEXT: 3
// BUF-NEXT: 4
