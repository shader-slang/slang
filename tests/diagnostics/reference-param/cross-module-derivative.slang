//TEST_IGNORE_FILE:

// Derivatives for functions with reference parameters from other modules, for
// `derivative-library.slang`. Each derivative is rejected, so this module fails to
// compile.

module "cross-module-derivative";

import "cross-module-primal";
__exported import "cross-module-exported-primal";

[ForwardDerivativeOf(referenceForwardPrimal)]
public DifferentialPair<float> referenceForwardDerivative(float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

[BackwardDerivativeOf(referenceBackwardPrimal)]
public void referenceBackwardDerivative(float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

[ForwardDerivativeOf(groupSharedForwardPrimal)]
public DifferentialPair<float> groupSharedForwardDerivative(float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

[BackwardDerivativeOf(groupSharedBackwardPrimal)]
public void groupSharedBackwardDerivative(float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

[ForwardDerivativeOf(exportedReferenceForwardPrimal)]
public DifferentialPair<float> exportedReferenceForwardDerivative(float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

[BackwardDerivativeOf(exportedGroupSharedBackwardPrimal)]
public void exportedGroupSharedBackwardDerivative(float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

[BackwardDerivativeOf(exportedReferenceBackwardPrimal)]
public void exportedReferenceBackwardDerivative(float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

[ForwardDerivativeOf(exportedGroupSharedForwardPrimal)]
public DifferentialPair<float> exportedGroupSharedForwardDerivative(float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}
