//TEST_IGNORE_FILE:

// Derivatives for functions that `__exported import` brings in, for `cross-module.slang`.

module "cross-module-exported-derivative";

__exported import "cross-module-exported-primal";

[ForwardDerivativeOf(exportedSplitForwardPrimal)]
public DifferentialPair<float> exportedSplitForwardDerivative(no_diff float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, (1000.0 + 2.0 * a * x.p) * x.d);
}

[BackwardDerivativeOf(exportedSplitBackwardPrimal)]
public void exportedSplitBackwardDerivative(no_diff float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * (1000.0 + 2.0 * a * x.p));
}
