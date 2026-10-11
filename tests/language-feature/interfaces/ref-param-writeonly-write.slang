// The callee can write through a `__ref_writeonly` parameter, including an array element and
// a forwarded `__ref_writeonly` argument, and the caller sees the writes.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

void store(__ref_writeonly int x, int value)
{
    x = value;
}

void forward(__ref_writeonly int x, int value)
{
    store(x, value + 1);
}

void storeElement(__ref_writeonly int a[2], int value)
{
    a[1] = value;
}

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int a = 0;
    store(a, 7);
    int b = 0;
    forward(b, 7);
    int arr[2] = { 0, 0 };
    storeElement(arr, 9);
    // CHECK: 7
    // CHECK: 8
    // CHECK: 9
    outputBuffer[0] = a;
    outputBuffer[1] = b;
    outputBuffer[2] = arr[1];
}
