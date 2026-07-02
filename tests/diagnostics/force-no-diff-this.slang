//TEST:SIMPLE(filecheck=SPIRV): -target spirv -stage compute -entry computeMain

struct MyStruct<T> where T : __BuiltinFloatingPointType
{
    float a;
    __init(float a) { this.a = a; }

    [Differentiable]
    T eval(T x)
    {
        return exp(x * T(a) * T(a));
    }

    [Differentiable]
    [NoDiffThis]
    T eval1(T x)
    {
        return exp(x * T(a) * T(a));
    }
};

[Differentiable]
float evalFunc(float x)
{
    MyStruct<float> s = {x};
    return s.eval(x) + s.eval1(x);
}

RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint id : SV_DispatchThreadID)
{
    var x = diffPair(2.0f);
    bwd_diff(evalFunc)(x, 1.0f);

    output[0] = x.d;
}

//SPIRV: OpEntryPoint GLCompute
