//TEST:SIMPLE(filecheck=CHECK):

// Regression test for shader-slang/slang#11473 (the minimal trigger). Part of the
// cross-`__include` `#pragma warning` family alongside `pragma-warning-multifile-*` and
// `nested-pragma-*`.
//
// Even an EMPTY `#pragma warning(push)/(pop)` in a later `__include`d file must not re-enable a
// root-level disable for an earlier file's code.

module pragma_emptypushpop_test;

namespace control
{
    interface IConvertibleFrom<From> {}

    // Positive control: BEFORE the disable below, so it MUST emit E30856 — guards the `CHECK-NOT`
    // from passing vacuously.
    extension<From : __BuiltinIntegerType, To : IConvertibleFrom<From>, let N : int>
    vector<To, N> : IConvertibleFrom<vector<From, N>> {}
}
// CHECK: E30856

#pragma warning(disable: 30856)

__include "pragma-warning-include-emptypushpop-body.slang";
__include "pragma-warning-include-emptypushpop-footer.slang";

// The body file's warning (after the disable) must stay suppressed, so no *second* E30856
// may appear after the positive control above.
// CHECK-NOT: E30856
