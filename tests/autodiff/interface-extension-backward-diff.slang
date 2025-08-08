//TEST:COMPILE:-target spirv -entry computeMain
//TEST:COMPILE:-target dxil-assembly -entry computeMain

// This test reproduces the crash scenario from issue #8054
// where [BackwardDifferentiable] on interface methods implemented via extensions
// would crash with "The key already exists in Dictionary" error.

interface IVector<T, int N> : IDifferentiable, IArithmetic
    where T : __BuiltinFloatingPointType {}

struct Vector<T, int N>
    where T : __BuiltinFloatingPointType
{
    T data[N];

    __init() {}
    __init(int value) 
    {
        [MaxIters(N)]
        for (int i = 0; i < N; i++)
            data[i] = T(value);
    }
    __init(This other) 
    {
        [MaxIters(N)]
        for (int i = 0; i < N; i++)
            data[i] = other.data[i];
    }

    __subscript(uint index) -> T
    {
        get { return data[index]; }
        set { data[index] = newValue; }
    }

    [BackwardDifferentiable] // This used to crash with extensions
    This add(This other)
    {
        [MaxIters(N)]
        for (int i = 0; i < N; i++)
            other[i] += this[i];

        return this;
    }

    This sub(This other) {}
    This mul(This other) {}
    This div(This other) {}
    This mod(This other) {}
    This neg() {}

    bool equals(This other) {}
    bool lessThan(This other) {}
    bool lessThanOrEquals(This other) {}
}

// This extension used to cause a crash when BackwardDifferentiable was used
extension<T, int N> Vector<T, N> : IVector<T, N>
    where T : __BuiltinFloatingPointType {}

[shader("compute")]
[numthreads(1,1,1)]
void computeMain()
{
    // Just test that compilation succeeds without crashing
}