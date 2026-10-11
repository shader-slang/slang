// Elements of a `RWStructuredBuffer` and of a local array are memory locations, so they can be
// passed to `__ref_readonly` and `__ref_writeonly` parameters. A `__ref` parameter can be
// forwarded to either restricted mode: the callee reads through the first and writes through
// the second, and the caller sees the write.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

struct P
{
    int a;
    int b;
}

int sum(__ref_readonly int a[4])
{
    return a[0] + a[1] + a[2] + a[3];
}

int readValue(__ref_readonly int x)
{
    return x;
}

void setB(__ref_writeonly P p)
{
    p.b = 7;
}

void store(__ref_writeonly int x, int value)
{
    x = value;
}

int forwardRead(__ref int x)
{
    return readValue(x);
}

void forwardWrite(__ref int x, int value)
{
    store(x, value);
}

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4):name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=8):name=pairBuffer
RWStructuredBuffer<P> pairBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int local[4] = { 5, 6, 7, 8 };
    // CHECK: 26
    outputBuffer[0] = sum(local);

    // CHECK: 9
    store(outputBuffer[1], 9);

    // CHECK: 2
    outputBuffer[2] = readValue(inputBuffer[1]);

    setB(pairBuffer[1]);
    // CHECK: 7
    outputBuffer[3] = pairBuffer[1].b;

    // CHECK: 3
    outputBuffer[4] = forwardRead(inputBuffer[2]);

    forwardWrite(local[3], 11);
    // CHECK: 11
    outputBuffer[5] = local[3];

    forwardWrite(inputBuffer[0], 12);
    // CHECK: 12
    outputBuffer[6] = inputBuffer[0];
}
