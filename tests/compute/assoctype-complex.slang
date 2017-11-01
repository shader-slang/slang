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

__generic<T:ISimple>
T.U.V test(T simple, T.U v0, T.U v1)
{
    return simple.add(v0, v1);
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Simple s;
    Val v0, v1;
	float outVal = test<Simple>(s, v0, v1); // == 1.0
	outputBuffer[0] = outVal;
}