//TEST_IGNORE_FILE:

// A function with a forward-derivative extension, for tests that use it from another module. An
// extension cannot be `public`, so it is not visible outside this module.

module "cross-module-extension-library";

public float libraryForwardExtended(no_diff float a, float x) { return a * x * x; }

__func_extension fwd_diff(libraryForwardExtended)(no_diff float a, DifferentialPair<float> x) -> DifferentialPair<float>
{
    return diffPair(a * x.p * x.p, (1000.0 + 2.0 * a * x.p) * x.d);
}
