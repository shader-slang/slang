//TEST:SIMPLE(filecheck=CHECK):

// Regression test for shader-slang/slang#11473 (the minimal trigger).
// Even an EMPTY `#pragma warning(push)/(pop)` in a later `__include`d file must not
// re-enable a root-level disable for an earlier file's code.

module pragma_emptypushpop_test;

#pragma warning(disable: 30856)

__include "pragma-warning-include-emptypushpop-body.slang";
__include "pragma-warning-include-emptypushpop-footer.slang";

// CHECK-NOT: E30856
