//TEST:SIMPLE(filecheck=CHECK_FAIL): -target spirv -entry computeMain -stage compute -DFAIL
//TEST:SIMPLE(filecheck=CHECK_PASS): -target spirv -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -slang -compute -shaderobj -output-using-type

interface ITwoParamGeneric<A, B>
{
    int getVal();
}

struct Foo<A, B> : ITwoParamGeneric<A, B>
{
    int val = 0;
    int getVal()
    {
       return val; 
    }
}

struct NotPrimitiveCastable
{
    double data;
}

extension<A, B> Foo<A,B> 
    where int(A)
#ifdef FAIL
    where NotPrimitiveCastable(B)
#else
    where float(B)
#endif
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
//CHECK: 3
    Foo<float, int> x = Foo<float, int>();
    x.setVal(3);
    outBuffer[0] = x.getVal();
}