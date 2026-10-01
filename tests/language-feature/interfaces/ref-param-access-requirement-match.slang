// A `const __ref` parameter has the `RefReadOnly` passing mode and a plain `__ref`
// parameter has `RefReadWrite`; a requirement is satisfied by a method whose
// parameter uses the same mode (shader-slang/slang#13339).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

interface IAccumulator
{
    int read(const __ref int a[4]);
    void write(__ref int a[4], int value);
}

struct Accumulator : IAccumulator
{
    int read(const __ref int a[4]) { return a[0] + a[1] + a[2] + a[3]; }
    void write(__ref int a[4], int value) { a[1] = value; }
}

int run<T : IAccumulator>(T t)
{
    int a[4] = { 1, 2, 3, 4 };
    t.write(a, 10);
    return t.read(a);
}

interface IReader
{
    int read(const __ref int a[4]);
}

struct First : IReader
{
    int read(const __ref int a[4]) { return a[0]; }
}

// The extra defaulted parameter makes `Scaled` satisfy `IReader.read` through a
// synthesized wrapper, whose parameter must keep the requirement's `RefReadOnly` mode.
struct Scaled : IReader
{
    int read(const __ref int a[4], int scale = 2) { return scale * a[3]; }
}

IReader makeReader(int id)
{
    if (id == 0)
        return First();
    return Scaled();
}

//TEST_INPUT:ubuffer(data=[0 1 0], stride=4):name=ids
RWStructuredBuffer<int> ids;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Accumulator acc;
    // CHECK: 18
    outputBuffer[0] = run(acc);

    int a[4] = { 5, 6, 7, 8 };
    // CHECK: 5
    outputBuffer[1] = makeReader(ids[0]).read(a);
    // CHECK: 16
    outputBuffer[2] = makeReader(ids[1]).read(a);
}
