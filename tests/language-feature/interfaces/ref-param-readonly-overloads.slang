// A type can define one method with a `__ref` parameter and another with a `__ref_readonly`
// parameter. They are two functions with distinct mangled names, and each interface requirement
// is satisfied by the method with its passing mode.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

interface IReadOnly
{
    int read(__ref_readonly int x);
}

interface IReadWrite
{
    int read(__ref int x);
}

struct Both : IReadOnly, IReadWrite
{
    int read(__ref_readonly int x) { return x + 1; }
    int read(__ref int x) { return x + 100; }
}

int viaReadOnly<T : IReadOnly>(T t, __ref int a) { return t.read(a); }
int viaReadWrite<T : IReadWrite>(T t, __ref int a) { return t.read(a); }

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int a = 1;
    Both b;
    // CHECK: 2
    outputBuffer[0] = viaReadOnly(b, a);
    // CHECK: 101
    outputBuffer[1] = viaReadWrite(b, a);
}
