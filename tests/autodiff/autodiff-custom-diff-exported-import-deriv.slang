//TEST_IGNORE_FILE:

// Derivative-provider module for the exported-import custom derivative test.

module "autodiff-custom-diff-exported-import-deriv";

public DifferentialPair<float> helper_fwd(DifferentialPair<float> x)
{
    return diffPair(x.p * x.p, 5.0 * x.p * x.d);
}
