//TEST:SIMPLE(filecheck=CHECK_FAIL): -target spirv -entry computeMain -stage compute -DFAIL
//TEST:SIMPLE(filecheck=CHECK_PASS): -target spirv -entry computeMain -stage compute
//TEST:COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

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
    void setVal(int dataIn)
    {
        val = dataIn;
    }
}

//TEST_INPUT: set outBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float4> outBuffer;

void computeMain()
{
//CHECK_FAIL: error 30027:{{.*}}'setVal'{{.*}}'Foo<float, int>'.
//CHECK_PASS: OpEntryPoint
//BUF: 3
#ifdef FAIL
    // fails since while expanding A and applying `where`,
    // we will find a `float`, not a `int`
    Foo<float, int> x = Foo<float, int>();
#else
    Foo<int, int> x = Foo<int, int>();
#endif
    x.setVal(3);
    outBuffer[0] = x.getVal();
}