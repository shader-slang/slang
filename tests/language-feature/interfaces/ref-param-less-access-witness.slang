//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -experimental-feature

// A method whose reference parameter needs less access than the requirement's grants satisfies
// the requirement through a synthesized witness that forwards the requirement's `__ref`
// parameter. A call through the interface reads or writes the caller's storage exactly as a
// direct call to the method would.

interface IReadWrite
{
    uint touch(__ref uint x);
}

struct ReadsOnly : IReadWrite
{
    uint touch(__ref_readonly uint x) { return x + 10; }
}

struct WritesOnly : IReadWrite
{
    uint touch(__ref_writeonly uint x)
    {
        x = 7;
        return 0;
    }
}

uint callTouch<T : IReadWrite>(T t, inout uint v)
{
    var local = v;
    let result = t.touch(local);
    v = local;
    return result;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint a = 5;
    outputBuffer[0] = callTouch(ReadsOnly(), a);
    outputBuffer[1] = a;
    uint b = 5;
    outputBuffer[2] = callTouch(WritesOnly(), b);
    outputBuffer[3] = b;
}

// CHECK: 15
// CHECK-NEXT: 5
// CHECK-NEXT: 0
// CHECK-NEXT: 7
