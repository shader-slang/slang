//TEST:INTERPRET(filecheck=CHECK):

interface IGenericDifferentiableMethod
{
    [Differentiable]
    static float something<TMarker>(float x);
}

struct GenericDifferentiableMethodImpl : IGenericDifferentiableMethod
{
    [Differentiable]
    static float something<TMarker>(float x)
    {
        return x * 2.0f;
    }
}

float readThroughGenericInterfaceMethod<TImpl : IGenericDifferentiableMethod>(float x)
{
    return TImpl.something<int>(x);
}

interface ITensor<T : IDifferentiable, let D : int> : IDifferentiable
{
    [Differentiable]
    T load<each TIndex>(TIndex indices)
        where TIndex == int
        where countof(TIndex) == D;

    __subscript<each TIndex>(TIndex indices) -> T
        where TIndex == int
        where countof(TIndex) == D
    {
        [Differentiable]
        get { return load(indices); }
    }
}

struct Tensor<T : IDifferentiable, let N : int> : IDifferentiable, ITensor<T, N>
    where T.Differential == T
{
    typealias Differential = Tensor<T, N>;

    T value;

    [Differentiable]
    T load<each TIndex>(TIndex indices)
        where TIndex == int
        where countof(TIndex) == N
    {
        return value;
    }

    [Differentiable]
    static Differential dzero()
    {
        Differential result;
        result.value = T.dzero();
        return result;
    }

    [Differentiable]
    static Differential dadd(Differential left, Differential right)
    {
        Differential result;
        result.value = T.dadd(left.value, right.value);
        return result;
    }
}

[Differentiable]
float readThroughDefaultSubscript(Tensor<float, 2> tensor)
{
    return tensor[0, 1] * tensor[1, 0];
}

RWStructuredBuffer<float> outputBuffer;

void main()
{
    let methodValue = readThroughGenericInterfaceMethod<GenericDifferentiableMethodImpl>(4.0f);

    Tensor<float, 2> tensor;
    tensor.value = 3.0f;

    var tangent = Tensor<float, 2>.dzero();
    tangent.value = 1.0f;
    let fwdPair = fwd_diff(readThroughDefaultSubscript)(diffPair(tensor, tangent));

    var pair = diffPair(tensor, Tensor<float, 2>.dzero());
    bwd_diff(readThroughDefaultSubscript)(pair, 1.0f);

    printf("%f\n", methodValue);
    printf("%f\n", fwdPair.p);
    printf("%f\n", fwdPair.d);
    printf("%f\n", pair.d.value);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Tensor<float, 2> tensor;
    tensor.value = 3.0f;

    var tangent = Tensor<float, 2>.dzero();
    tangent.value = 1.0f;
    let fwdPair = fwd_diff(readThroughDefaultSubscript)(diffPair(tensor, tangent));

    var pair = diffPair(tensor, Tensor<float, 2>.dzero());
    bwd_diff(readThroughDefaultSubscript)(pair, 1.0f);

    outputBuffer[0] = fwdPair.d + pair.d.value;
}

// CHECK: 8.000000
// CHECK: 9.000000
// CHECK: 6.000000
// CHECK: 6.000000
