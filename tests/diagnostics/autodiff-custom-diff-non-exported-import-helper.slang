//TEST_IGNORE_FILE:

// Helper module used by the non-exported-import diagnostic regression.

module "autodiff-custom-diff-non-exported-import-helper";

public DifferentialPair<float> helper_fwd(DifferentialPair<float> x)
{
    return diffPair(x.p * x.p, 2.0 * x.p * x.d);
}

public void helper_bwd(inout DifferentialPair<float> x, float.Differential dOut)
{
    x = diffPair(x.p, 2.0 * x.p * dOut);
}
