//TEST_IGNORE_FILE:

// Intermediate module that re-exports the derivative module and exposes
// a public primal function with a custom forward derivative.

module "autodiff-custom-diff-exported-import-lib";

__exported import "autodiff-custom-diff-exported-import-deriv";

[ForwardDerivative(helper_fwd)]
public float exportedSquare(float x)
{
    return x * x;
}
