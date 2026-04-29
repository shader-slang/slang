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

    [ForwardDerivative(diffInternalCustom)]
    public static float publicCustomSquareWithInternalDerivative(float x)
    {
        return x * x;
    }

    internal static DifferentialPair<float> diffInternalCustom(DifferentialPair<float> x)
    {
        return diffPair(x.p * x.p, 9.0 * x.p * x.d);
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
        return fwd_diff(VisibilityCase.publicSquare)(diffPair(x, 1.0)).d;
    }

    public static float callInternalFwd(float x)
    {
        return fwd_diff(VisibilityCase.internalCube)(diffPair(x, 1.0)).d;
    }

    public static float callPrivateFwd(float x)
    {
        return fwd_diff(VisibilityCase.privatePow4)(diffPair(x, 1.0)).d;
    }

    public static float callPrivateCustomFwd(float x)
    {
        return fwd_diff(VisibilityCase.privateCustomSquare)(diffPair(x, 1.0)).d;
    }

    public static float callPublicCustomInternalFwd(float x)
    {
        return fwd_diff(VisibilityCase.publicCustomSquareWithInternalDerivative)(diffPair(x, 1.0))
            .d;
    }

    public static float callPrivateExtensionFwd(float x)
    {
        return fwd_diff(VisibilityCase.privateExtensionSquare)(diffPair(x, 1.0)).d;
    }

    public static float callPublicBwd(float x)
    {
        var dx = diffPair(x);
        bwd_diff(VisibilityCase.publicCube)(dx, 1.0);
        return dx.d;
    }

    public static float callInternalBwd(float x)
    {
        var dx = diffPair(x);
        bwd_diff(VisibilityCase.internalPow4)(dx, 1.0);
        return dx.d;
    }

    public static float callPrivateBwd(float x)
    {
        var dx = diffPair(x);
        bwd_diff(VisibilityCase.privatePow5)(dx, 1.0);
        return dx.d;
    }

    public static float callPrivateCustomBwd(float x)
    {
        var dx = diffPair(x);
        bwd_diff(VisibilityCase.privateCustomCube)(dx, 1.0);
        return dx.d;
    }
}

extension VisibilityCase
{
    [ForwardDifferentiable]
    private static float privateExtensionSquare(float x)
    {
        return x * x;
    }
}
