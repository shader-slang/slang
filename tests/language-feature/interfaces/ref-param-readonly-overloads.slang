// A type can define methods that differ only in whether a parameter is `__ref`, `__ref_readonly`,
// or `__ref_writeonly`. They are distinct functions with distinct mangled names, and each
// interface requirement is satisfied by the method with its passing mode.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -experimental-feature
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -xslang -experimental-feature

interface IReadOnly
{
    int read(__ref_readonly int x);
}

interface IWriteOnly
{
    void write(__ref_writeonly int x);
}

interface IReadWrite
{
    int read(__ref int x);
    void write(__ref int x);
}

struct Both : IReadOnly, IWriteOnly, IReadWrite
{
    int read(__ref_readonly int x) { return x + 1; }
    int read(__ref int x) { return x + 100; }
    void write(__ref_writeonly int x) { x = 3; }
    void write(__ref int x) { x = x + 40; }
}

int viaReadOnly<T : IReadOnly>(T t, __ref int a) { return t.read(a); }
int viaReadWrite<T : IReadWrite>(T t, __ref int a) { return t.read(a); }

int viaWriteOnly<T : IWriteOnly>(T t, __ref int a)
{
    t.write(a);
    return a;
}

int viaReadWriteWrite<T : IReadWrite>(T t, __ref int a)
{
    t.write(a);
    return a;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int a = 1;
    int b = 1;
    int c = 1;
    Both both;
    // CHECK: 2
    outputBuffer[0] = viaReadOnly(both, a);
    // CHECK: 101
    outputBuffer[1] = viaReadWrite(both, a);
    // CHECK: 3
    outputBuffer[2] = viaWriteOnly(both, b);
    // CHECK: 41
    outputBuffer[3] = viaReadWriteWrite(both, c);
}
