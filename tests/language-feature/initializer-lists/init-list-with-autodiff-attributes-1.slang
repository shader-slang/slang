//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

struct TestInner : IDifferentiable
{
    typealias Differential = TestInner;
    int a;
}
struct Test : IDifferentiable
{
    uint a;
    uint b;
    TestInner test;

    [Differentiable]
    __init(uint data1, uint data2, TestInner data)
    {
        a = data1;
        b = data2;
        test = {};
    }
};

struct Test2 : IDifferentiable
{
    typealias Differential = Test2;
    TestInner a;
}

//CHECK: computeMain

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Test.Differential test = { {} };

    outputBuffer[0] = true
        && test.test.a == 0
        ;
}