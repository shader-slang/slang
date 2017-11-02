//TEST(smoke, compute):COMPARE_COMPUTE:-xslang -use-ir
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):dxbinding(0),glbinding(0),out

RWStructuredBuffer<float> outputBuffer;
interface IBase
{
     associatedtype V;
     V sub(V a0, V a1);
}
interface ISimple
{
    associatedtype U : IBase;
    U.V add(U v0, U v1);
}

struct Val : IBase
{
    typedef int V;
    V sub(V a0, V a1)
    {
        return a0-a1;
    }
};

struct Simple : ISimple
{
    typedef Val U;
    Val.V add(U v0, U v1)
    {
        return v0.sub(4, v1.sub(1,2));
    }
};
/*
__generic<T:ISimple>
T.U.V test(T simple, T.U v0, T.U v1)
{
    return simple.add(v0, v1);
}

__generic<T:__BuiltinArithmeticType>
T test(T v0, T v1)
{
    return v0 + v1;
}
*/
__generic<T:__BuiltinFloatingPointType>
T test(T v0, T v1)
{
    return T(3.0);
}
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    //Simple s;
    //Val v0, v1;
	//float outVal = test(s, v0, v1); // == 1.0
    float outVal = test<float>(1.0, 2.0);
	outputBuffer[dispatchThreadID.x] = outVal;
}