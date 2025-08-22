//DIAGNOSTIC_TEST:SIMPLE:
// Just make sure this test won't crash.
// See: https://github.com/shader-slang/slang/issues/8068

interface IVector<T, int N> : IDifferentiable
    where T : __BuiltinFloatingPointType
{
    [BackwardDifferentiable]
    T sum();
}

struct InlineVector<T, int N> :  IVector<T, N>
    where T : __BuiltinFloatingPointType
{
    typealias Differential = This;

    T[N] data;

    __init(T value) { data[0] = value; data[1] = value;}
    __init(T[N] value) { data = value; }

    [BackwardDifferentiable]
    T sum()
    {
        return data[0] + data[1];
    }
}

RWStructuredBuffer<float> g_output;

[BackwardDifferentiable]
float wrapper(InlineVector<float, 2> input)
{
    return input.sum();
}

[shader("compute")]
void computeMain(uint3 thread_id : SV_DispatchThreadID)
{
    float[2] values = { 1.0, 2.0 };
    var di = diffPair(InlineVector<float, 2>(values));
    bwd_diff(wrapper)(di, 1.0f);
    g_output[0] = di.d.data[0];
    g_output[1] = di.d.data[1];
}
