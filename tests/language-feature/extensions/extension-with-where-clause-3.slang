//TEST:SIMPLE(filecheck=CHECK_FAIL): -target spirv -entry computeMain -stage compute -DFAIL
//TEST:SIMPLE(filecheck=CHECK_PASS): -target spirv -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -slang -compute -shaderobj -output-using-type

struct Foo<each A>
{
    int val = 0;
    int getVal()
    {
       return val; 
    }
}

extension<each A> Foo<A>
    where A == int
{
    [mutating]
    void setVal1(int dataIn)
    {
        val += dataIn;
    }
}

//TEST_INPUT: set outBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float4> outBuffer;

void computeMain()
{
//CHECK_FAIL: error 30027: 'setVal1'{{.*}}'Foo<float>'
//CHECK_PASS: OpEntryPoint
//CHECK: 3
#ifdef FAIL
    // fails since while expanding A and applying `where`,
    // we will find a `float`, not a `int`
    Foo<float> x = Foo<float>();
#else
    Foo<int> x = Foo<int>();
#endif
    x.setVal1(3);
    outBuffer[0] = x.getVal();
}