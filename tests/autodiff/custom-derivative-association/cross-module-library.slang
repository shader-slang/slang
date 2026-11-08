//TEST_IGNORE_FILE:

// Functions declared together with their derivatives, for `cross-module.slang`.

module "cross-module-library";

[Differentiable]
public float librarySynthesized(no_diff float a, float x) { return a * x * x; }

[ForwardDerivative(libraryForwardNamedDerivative)]
public float libraryForwardNamed(no_diff float a, float x) { return a * x * x; }

public DifferentialPair<float> libraryForwardNamedDerivative(no_diff float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, (1000.0 + 2.0 * a * x.p) * x.d);
}

[BackwardDerivative(libraryBackwardNamedDerivative)]
public float libraryBackwardNamed(no_diff float a, float x) { return a * x * x; }

public void libraryBackwardNamedDerivative(no_diff float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * (1000.0 + 2.0 * a * x.p));
}

public float libraryForwardOfPrimal(no_diff float a, float x) { return a * x * x; }

[ForwardDerivativeOf(libraryForwardOfPrimal)]
public DifferentialPair<float> libraryForwardOfDerivative(no_diff float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, (1000.0 + 2.0 * a * x.p) * x.d);
}

public float libraryBackwardOfPrimal(no_diff float a, float x) { return a * x * x; }

[BackwardDerivativeOf(libraryBackwardOfPrimal)]
public void libraryBackwardOfDerivative(no_diff float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * (1000.0 + 2.0 * a * x.p));
}
