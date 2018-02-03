//TEST(compute):COMPARE_COMPUTE:
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out

RWStructuredBuffer<float> outputBuffer;

interface IAdd
{
    float addf(float u, float v);
}

interface ISub
{
    float subf(float u, float v);
}

interface IAddAndSub : IAdd, ISub
{
}

struct Simple : IAddAndSub
{
    float addf(float u, float v)
    {
        return u+v;
    }
    float subf(float u, float v)
    {
        return u-v;
    }
};

float testAdd<T:IAdd>(T t)
{
    return t.addf(1.0, 1.0);
}

interface IAssoc
{
    associatedtype AT : IAdd;
}

struct AssocImpl : IAssoc
{
    typedef Simple AT;
};

float testAdd2<T:IAssoc>(T assoc)
{
    T.AT obj;
    return obj.addf(1.0, 1.0);
}

float testSub<T:ISub>(T t, float base)
{
    return t.subf(base, 1.0);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    AssocImpl s;
	float outVal = testAdd2(s);
    Simple s1;
    outVal += testSub(s1, outVal);
	outputBuffer[dispatchThreadID.x] = outVal;
}