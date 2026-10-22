// `const` on a by-value `in` parameter only stops the body from assigning to it, so it is not part
// of the function's signature: `scale(int)` and `scale(const int)` declare one function. On a
// `__ref` parameter, `const` makes the effective passing mode `RefReadOnly`, so `read(__ref int)`
// and `read(const __ref int)` are two functions, and each interface requirement is satisfied by
// the method with its passing mode.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

int scale(int x);
int scale(const int x) { return x * 3; }

interface IReadOnly
{
    int read(const __ref int x);
}

interface IReadWrite
{
    int read(__ref int x);
}

struct Both : IReadOnly, IReadWrite
{
    int read(const __ref int x) { return x + 1; }
    int read(__ref int x) { return x + 100; }
}

int viaReadOnly<T : IReadOnly>(T t, __ref int a) { return t.read(a); }
int viaReadWrite<T : IReadWrite>(T t, __ref int a) { return t.read(a); }

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int a = 1;
    Both b;
    // CHECK: 6
    outputBuffer[0] = scale(2);
    // CHECK: 2
    outputBuffer[1] = viaReadOnly(b, a);
    // CHECK: 101
    outputBuffer[2] = viaReadWrite(b, a);
}
