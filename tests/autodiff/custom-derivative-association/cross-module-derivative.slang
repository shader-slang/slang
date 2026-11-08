//TEST_IGNORE_FILE:

// A derivative for a function that a plain `import` brings in, for `cross-module.slang`.

module "cross-module-derivative";

import "cross-module-primal";

[ForwardDerivativeOf(splitForwardPrimal)]
public DifferentialPair<float> splitForwardDerivative(no_diff float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, (1000.0 + 2.0 * a * x.p) * x.d);
}
