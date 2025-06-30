implementing mlp;

// A wrapper of CoopVec<T> to allow it being used in differentiable context.
//
public struct MLVec<int N> : IDifferentiable
{
    public CoopVec<NFloat, N> data;
    public typealias Differential = MLVec<N>;

    public static MLVec<N> fromArray(NFloat[N] values)
    {
        MLVec<N> result;
        [ForceUnroll]
        for (int i = 0; i < N; i++)
            result.data[i] = values[i];
        return result;
    }

    internal static NFloat[N] coopVecToArray(CoopVec<NFloat, N> v)
    {
        NFloat[N] arr;
        [ForceUnroll]
        for (int i = 0; i < N; i++)
            arr[i] = v[i];
        return arr;
    }

    [BackwardDerivativeOf(fromArray)]
    internal static void fromArrayBwd(inout DifferentialPair<NFloat[N]> values, MLVec<N> dResult)
    {
        values = diffPair(values.p, coopVecToArray(dResult.data));
    }

    internal static NFloat[N] toArray(MLVec<N> vec)
    {
        return coopVecToArray(vec.data);
    }

    [BackwardDerivativeOf(toArray)]
    internal static void toArrayBwd(inout DifferentialPair<MLVec<N>> vec, NFloat[N] dResult)
    {
        vec = diffPair(vec.p, MLVec<N>.fromArray(dResult));
    }

    [Differentiable]
    public NFloat[N] toArray()
    {
        return toArray(this);
    }

    public override static Differential dadd(Differential d0, Differential d1)
    {
        return {d0.data + d1.data};
    }
    public override static Differential dmul<U:__BuiltinRealType>(U s, Differential d)
    {
        return {d.data * __realCast<NFloat>(s)};
    }
    public override static Differential dzero()
    {
        return {};
    }
}
