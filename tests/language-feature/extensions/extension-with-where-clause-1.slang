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

extension<A, B> Foo<A,B> where A == B
{
    [mutating]
    void setVal(int dataIn)
    {
        val = dataIn;
    }
}

void test<A, B>(Foo<A,B> x) where A == B
{
}

//TEST_INPUT: set outBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float4> outBuffer;

void computeMain()
{
//CHECK_FAIL-DAG: error 30027:{{.*}}'setVal'{{.*}}'Foo<int, float>'.
//CHECK_FAIL-DAG: error 39999: could not specialize generic for arguments of type
//CHECK_PASS: OpEntryPoint
//CHECK: 3
#ifdef FAIL
    Foo<int, float> x = Foo<int, float>();
#else
    Foo<int, int> x = Foo<int, int>();
#endif
    x.setVal(3);
    test(x);
    outBuffer[0] = x.getVal();
}