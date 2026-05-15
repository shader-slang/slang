//TEST_IGNORE_FILE:

// Derivative-provider module for the exported-import custom derivative test.

module "autodiff-custom-diff-exported-import-deriv";

public DifferentialPair<float> helper_fwd(DifferentialPair<float> x)
{
    return diffPair(x.p * x.p, 5.0 * x.p * x.d);
}

public void helper_bwd(inout DifferentialPair<float> x, float.Differential dOut)
{
    x = diffPair(x.p, 7.0 * x.p * dOut);
}
