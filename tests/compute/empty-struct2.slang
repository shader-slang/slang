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

struct Empty : IInterface
{
    struct T{};
    T value;
    T getT()
    {
        T t;
        return t;
    }
    [mutating]
    void setT(T val)
    {
        value = val;
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
    Empty obj;
    test(obj);
    outputBuffer[dispatchThreadID.x] = dispatchThreadID.x;
}