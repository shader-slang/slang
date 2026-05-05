//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Verifies that a public custom derivative is rejected when it points at
// a helper imported from another module without `__exported import`.

module "autodiff-custom-diff-non-exported-import";

import "autodiff-custom-diff-non-exported-import-helper";

[ForwardDerivative(helper_fwd)]
/*CHECK:
 ^^^^^^^^^^^^^^^^^ custom derivative uses non-exported import
 ^^^^^^^^^^^^^^^^^ a publicly visible custom derivative cannot reference 'helper_fwd' from a module that is imported but not re-exported with '__exported import'.
*/
public float publicFwd(float x)
{
    return x * x;
}

[BackwardDerivative(helper_bwd)]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^ custom derivative uses non-exported import
 ^^^^^^^^^^^^^^^^^^ a publicly visible custom derivative cannot reference 'helper_bwd' from a module that is imported but not re-exported with '__exported import'.
*/
public float publicBwd(float x)
{
    return x * x;
}
