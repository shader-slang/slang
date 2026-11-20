//TEST:COMPARE_COMPUTE(targets=all-dx11-dx12-wgpu, filecheck-buffer=CHECK): -output-using-type

interface IFoo
{
    int getVal();
}

struct Foo : IFoo
{
    int val;
    int getVal() { return val; }
}

struct Bar : IFoo
{
    float val;
    int getVal() { return (int)val + 1; }
}

//TEST_INPUT: set pFoo = ubuffer(data=[0 0 2 0 2.0f], stride=4);
//TEST_INPUT: type_conformance Foo:IFoo = 1;
//TEST_INPUT: type_conformance Bar:IFoo = 2;
uniform IFoo* pFoo;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4);
RWStructuredBuffer<float> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    // CHECK: 3.0
    outputBuffer[0] = pFoo->getVal();
}