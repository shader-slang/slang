//TEST(smoke,compute):COMPARE_COMPUTE:
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out
// This is a basic test for Slang compute shader.

RWStructuredBuffer<uint> outputBuffer;

interface IInterface
{
    associatedtype T;
    T getT();
    [mutating]
    void setT(T val);
}
interface IEmptyS
{
    float emptyFunc();
};
struct EmptyS : IEmptyS
{
    float emptyFunc() {return 0.0;}    
};

struct Empty<TT : IEmptyS> : IInterface
{
    typedef TT T;
    TT value;
    float a;
    TT getT()
    {
        return value;
    }
    [mutating]
    void setT(TT val)
    {
        value = val;
        a = value.emptyFunc();
    }
}

void test<Obj : IInterface>(Obj obj)
{
    Obj.T v = obj.getT();
    obj.setT(v);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Empty<EmptyS> obj;
    test(obj);
    outputBuffer[dispatchThreadID.x] = dispatchThreadID.x;
}