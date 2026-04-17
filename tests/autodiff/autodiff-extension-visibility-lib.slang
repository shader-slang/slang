//TEST_IGNORE_FILE:

module "autodiff-extension-visibility-lib";

public struct VisibilityCase
{
    [ForwardDifferentiable]
    public static float publicSquare(float x)
    {
        return x * x;
    }

    [ForwardDifferentiable]
    internal static float internalCube(float x)
    {
        return x * x * x;
    }

    [ForwardDifferentiable]
    private static float privatePow4(float x)
    {
        return x * x * x * x;
    }

    [ForwardDerivative(diffPrivateCustom)]
    private static float privateCustomSquare(float x)
    {
        return x * x;
    }

    private static DifferentialPair<float> diffPrivateCustom(DifferentialPair<float> x)
    {
        return diffPair(x.p * x.p, 5.0 * x.p * x.d);
    }

    [BackwardDifferentiable]
    public static float publicCube(float x)
    {
        return x * x * x;
    }

    [BackwardDifferentiable]
    internal static float internalPow4(float x)
    {
        return x * x * x * x;
    }

    [BackwardDifferentiable]
    private static float privatePow5(float x)
    {
        return x * x * x * x * x;
    }

    [BackwardDerivative(diffPrivateCustomBwd)]
    private static float privateCustomCube(float x)
    {
        return x * x * x;
    }

    private static void diffPrivateCustomBwd(
        inout DifferentialPair<float> x,
        float.Differential dOut)
    {
        x = diffPair(x.p, 7.0 * x.p * dOut);
    }

    public static float callPublicFwd(float x)
    {
        return VisibilityCase.publicSquare.fwd_diff(diffPair(x, 1.0)).d;
    }

    public static float callInternalFwd(float x)
    {
        return VisibilityCase.internalCube.fwd_diff(diffPair(x, 1.0)).d;
    }

    public static float callPrivateFwd(float x)
    {
        return VisibilityCase.privatePow4.fwd_diff(diffPair(x, 1.0)).d;
    }

    public static float callPrivateCustomFwd(float x)
    {
        return VisibilityCase.privateCustomSquare.fwd_diff(diffPair(x, 1.0)).d;
    }

    public static float callPublicBwd(float x)
    {
        var dx = diffPair(x);
        VisibilityCase.publicCube.bwd_diff(dx, 1.0);
        return dx.d;
    }

    public static float callInternalBwd(float x)
    {
        var dx = diffPair(x);
        VisibilityCase.internalPow4.bwd_diff(dx, 1.0);
        return dx.d;
    }

    public static float callPrivateBwd(float x)
    {
        var dx = diffPair(x);
        VisibilityCase.privatePow5.bwd_diff(dx, 1.0);
        return dx.d;
    }

    public static float callPrivateCustomBwd(float x)
    {
        var dx = diffPair(x);
        VisibilityCase.privateCustomCube.bwd_diff(dx, 1.0);
        return dx.d;
    }
}
