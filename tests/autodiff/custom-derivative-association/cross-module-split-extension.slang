//TEST_IGNORE_FILE:

// Forward-derivative extensions for functions from other modules, for `func-extension-split.slang`.
// An extension cannot be `public`, so they are not visible outside this module.

module "cross-module-split-extension";

import "cross-module-primal";
__exported import "cross-module-exported-primal";

__func_extension fwd_diff(splitForwardExtended)(no_diff float a, DifferentialPair<float> x) -> DifferentialPair<float>
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

__func_extension fwd_diff(exportedSplitForwardExtended)(no_diff float a, DifferentialPair<float> x) -> DifferentialPair<float>
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}
