//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type

interface IBar 
{
    float get();
}

struct Bar : IBar 
{
    float get() {
        return 2;
    }
};

interface IFoo 
{
    property IBar b;
}

struct Foo : IFoo 
{
    IBar b;
};

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute"), numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    IBar b = Bar();
    Foo pc = Foo(b);
    outputBuffer[id.x] = pc.b.get(); // CHECK: 2
}