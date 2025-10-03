implementing neural;

// Vector interface
public interface IVector<T, int N> : IDifferentiable
    where T : __BuiltinFloatingPointType
    where T.Differential : __BuiltinFloatingPointType
{
    public static const int Size;

    // Override the Differential type to add conformance constraints
    public associatedtype Differential : IVector<T.Differential, N>;

    public __init();
    public __init(T value);
    public __init(T[N] data);
    public __init(This other);

    __subscript(int index)->T
    {
        get;
        set;
    }

    [BackwardDifferentiable]
    public OutputVector eval<int OutputSize, bool Bias, Storage, OutputVector>(
        Storage storage,
        no_diff Storage.Address address)
            where Storage : IStorage<T>
            where Storage.Differential : IStorage<T.Differential>
            where Storage.Address == Storage.Differential.Address
            where OutputVector : IVector<T, OutputSize>;

    [BackwardDifferentiable]
    public OutputVector eval<int OutputSize, BindlessStorage, OutputVector>(
        BindlessStorage.Address parameters)
            where BindlessStorage : IStorage<T>
            where BindlessStorage.Address : IPointerLikeAddress<T>
            where BindlessStorage.Address.Differential : IPointerLikeAddress<T.Differential>
            where OutputVector : IVector<T, OutputSize>;
}

public struct InlineVector<T, int N> : IVector<T, N>
    where T : __BuiltinFloatingPointType
    where T.Differential : __BuiltinFloatingPointType
{
    public typealias Differential = InlineVector<T.Differential, N>;
    public static const int Size = N;

    [DerivativeMember(Differential.m_data)]
    internal T[N] m_data;

    public __init() { m_data = {}; }
    public __init(T value) {
        [ForceUnroll]
        for (int i = 0; i < N; i++)
            m_data[i] = value;
    }

    public __init(T[N] data) { this.m_data = data; }
    public __init(This other) { this.m_data = other.m_data; }

    public __subscript(int index) -> T
    {
        get() { return this.m_data[index]; }
        set() { this.m_data[index] = newValue; }
    }

    [BackwardDerivative(evalBwd)]
    public OutputVector eval<int OutputSize, bool Bias, Storage, OutputVector>(
        Storage storage,
        no_diff Storage.Address address)
        where Storage : IStorage<T>
        where Storage.Differential : IStorage<T.Differential>
        where Storage.Address == Storage.Differential.Address
        where OutputVector : IVector<T, OutputSize>
    {
        OutputVector output = OutputVector();

        if (Bias)
        {
            let biasOffset = Storage.getOffset(address, N * OutputSize);
            [ForceUnroll]
            for (int i = 0; i < OutputSize; i++)
            {
                let biasAddress = Storage.getOffset(biasOffset, i);
                output[i] = storage.read(biasAddress);
            }
        }

        // TODO: transpose if N >> OutputSize or OutputSize < 4
        let weightSliceAddress = address;
        [MaxIters(N)]
        for (int j = 0; j < N; j++)
        {
            let x = m_data[j];

            [ForceUnroll]
            for (int i = 0; i < OutputSize; i++)
            {
                let elementOffset = Storage.getOffset(address, j * OutputSize + i);
                output[i] += x * storage.read(elementOffset);
            }
        }

        return output;
    }

    static void evalBwd<int OutputSize, bool Bias, Storage, OutputVector>(
        inout DifferentialPair<This> dthis,
        DifferentialPtrPair<Storage> dstorage,
        no_diff Storage.Address address,
        OutputVector.Differential doutput)
            where Storage : IStorage<T>
            where Storage.Differential : IStorage<T.Differential>
            where Storage.Address == Storage.Differential.Address
            where OutputVector : IVector<T, OutputSize>
    {
        // Derivative of the input is transposed weight matrix times the output differential
        var d = dthis.d;

        [MaxIters(OutputSize)]
        for (int j = 0; j < OutputSize; j++)
        {
            T.Differential dy = doutput[j];

            [ForceUnroll]
            for (int i = 0; i < N; i++)
            {
                Storage.Address elementOffset = Storage.getOffset(address, i * OutputSize + j);
                T.Differential prod = T.Differential.dmul(dstorage.p.read(elementOffset), dy);
                d[i] = T.Differential.dadd(d[i], prod);
            }
        }

        // Derivative of the weights is the outer product of the input and the output differential
        [MaxIters(N)]
        for (int i = 0; i < N; i++)
        {
            let x = dthis.p[i];
            [ForceUnroll]
            for (int j = 0; j < OutputSize; j++)
            {
                T.Differential dy = doutput[j];
                let elementOffset = Storage.getOffset(address, i * OutputSize + j);
                T.Differential prod = T.Differential.dmul(x, dy);
                Storage.Differential diffStorage = dstorage.d;
                diffStorage.atomicAdd(elementOffset, prod);
            }
        }

        // Derivative of the bias is the same as the output differential
        if (Bias)
        {
            [ForceUnroll]
            for (int j = 0; j < OutputSize; j++)
            {
                let biasOffset = Storage.getOffset(address, N * OutputSize + j);
                dstorage.d.atomicAdd(biasOffset, doutput[j]);
            }
        }

        dthis = DifferentialPair<This>(dthis.p, d);
    }

    // Apply from bindless storage
    [BackwardDerivative(evalBwd)]
    public OutputVector eval<int OutputSize, BindlessStorage, OutputVector>(
        BindlessStorage.Address parameters)
            where BindlessStorage : IStorage<T>
            where BindlessStorage.Address : IPointerLikeAddress<T>
            where BindlessStorage.Address.Differential : IPointerLikeAddress<T.Differential>
            where OutputVector : IVector<T, OutputSize>
    {
        var output = OutputVector();

        let biases = parameters.getOffset(N * OutputSize);

        [ForceUnroll]
        for (int i = 0; i < OutputSize; i++)
            output[i] = biases[i];

        [MaxIters(N)]
        for (int j = 0; j < N; j++)
        {
            let x = m_data[j];
            [ForceUnroll]
            for (int i = 0; i < OutputSize; i++)
                output[i] += x * parameters[j * OutputSize + i];
        }

        return output;
    }

    static public void evalBwd<int OutputSize, BindlessStorage, OutputVector>(
        inout DifferentialPair<This> dthis,
        DifferentialPtrPair<BindlessStorage.Address> dparameters,
        OutputVector.Differential doutput)
      where BindlessStorage : IStorage<T>
      where BindlessStorage.Address : IPointerLikeAddress<T>
      where BindlessStorage.Address.Differential : IPointerLikeAddress<T.Differential>
      where OutputVector : IVector<T, OutputSize>
      where OutputVector.Differential : IVector<T.Differential, OutputSize>
    {
        // Derivative of the input is transposed weight matrix times the output differential
        var d = dthis.d;
        [MaxIters(OutputSize)]
        for (int j = 0; j < OutputSize; j++)
        {
            let dy = doutput[j];
            [ForceUnroll]
            for (int i = 0; i < N; i++)
            {
                T.Differential prod = T.Differential.dmul(dparameters.p[i * OutputSize + j], dy);
                d[i] = T.Differential.dadd(d[i], prod);
            }
        }

        // Derivative of the weights is the outer product of the input and the output differential
        [MaxIters(N)]
        for (int i = 0; i < N; i++)
        {
            let x = dthis.p[i];
            let weightSlice = dparameters.d.getOffset(i * OutputSize);
            [ForceUnroll]
            for (int j = 0; j < OutputSize; j++)
            {
                let dy = doutput[j];
                T.Differential prod = T.Differential.dmul(x, dy);
                weightSlice.atomicAdd(j, prod);
            }
        }

        let biasAddress = dparameters.d.getOffset(N * OutputSize);
        [ForceUnroll]
        for (int j = 0; j < OutputSize; j++)
        {
            biasAddress.atomicAdd(j, doutput[j]);
        }

        dthis = DifferentialPair<This>(dthis.p, d);
    }
}
