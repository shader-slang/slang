// Non-test helper module for matrix-layout-late-specialize.slang (no //TEST: directive).
// Cloned into the linked module only when Thing<float> is specialized from the conformance,
// so its matrix<T, R, C> mentions keep the defaulted (unknown) layout until then.
module "matrix-layout-late-specialize-module";

public interface IAcc
{
    static void acc(This* buf, int idx, This v);
}

public extension<T : __BuiltinFloatingPointType, let R : int, let C : int> matrix<T, R, C> : IAcc
{
    public static void acc(matrix<T, R, C>* buf, int idx, matrix<T, R, C> v)
    {
        T* p = (T*)(buf + idx);
        [ForceUnroll]
        for (int r = 0; r < R; ++r)
            [ForceUnroll]
            for (int c = 0; c < C; ++c)
                p[r * C + c] = v[r][c];
    }
}

public struct DTensor<T : IDifferentiable & IAcc>
    where T.Differential : IAcc
{
    public T* primal;
    public T.Differential* grad;

    [BackwardDerivative(get_bwd)]
    public T get(int i)
    {
        return primal[i];
    }

    public void get_bwd(int i, T.Differential v)
    {
        T.Differential::acc(grad, i, v);
    }
}

[anyValueSize(32)]
public dyn interface IThing
{
    [Differentiable]
    float eval(no_diff int i, float4 v);
}

public struct Thing<Real : __BuiltinFloatingPointType> : IThing
{
    DTensor<matrix<Real, 4, 4> > t;

    [Differentiable]
    public float eval(no_diff int i, float4 v)
    {
        vector<Real, 4> vr = { Real(v.x), Real(v.y), Real(v.z), Real(v.w) };
        return __realCast<float, Real>(mul(vr, t.get(i)).x);
    }
}
