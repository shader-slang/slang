interface IVector<T, int N> : IDifferentiable
    where T : __BuiltinFloatingPointType
{
    [BackwardDifferentiable]
    T sum();
}

struct InlineVector<T, int N> : IVector<T, N>
    where T : __BuiltinFloatingPointType
{
    typealias Differential = This;

    T[N] data;

    __init(T value) { data[0] = value; }

    [BackwardDifferentiable]
    T sum()
    {
        T sum = T(0.0);

        [MaxIters(N)]
        for (int i = 0; i < N; i++)
            sum += data[i];

        return sum;
    }
}

[shader("compute")]
void main()
{
    var input = InlineVector<float, 2>(0.0);
    float result = input.sum();
}