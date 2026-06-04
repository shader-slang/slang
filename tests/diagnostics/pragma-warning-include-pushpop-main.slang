//TEST:SIMPLE(filecheck=CHECK):

// Regression test for shader-slang/slang#11473. Part of the cross-`__include`
// `#pragma warning` family alongside `pragma-warning-multifile-*` and `nested-pragma-*`.
//
// A root-level `#pragma warning(disable)` must keep suppressing the warning in an earlier
// `__include`d file even when a *later* `__include`d file performs an unrelated
// `#pragma warning(push)/(pop)`. The push/pop in the later file must not re-enable the
// root-level disable for the earlier file's code.

module pragma_pushpop_test;

namespace control
{
    interface IConvertibleFrom<From> {}

    // Positive control: this extension is BEFORE the disable below, so it MUST emit E30856.
    // It guards the `CHECK-NOT` from passing vacuously (e.g. if 30856 ever stops being emitted
    // for an unrelated reason, this CHECK fails loudly instead).
    extension<From : __BuiltinIntegerType, To : IConvertibleFrom<From>, let N : int>
    vector<To, N> : IConvertibleFrom<vector<From, N>> {}
}
// CHECK: E30856

#pragma warning(disable: 30856)

__include "pragma-warning-include-pushpop-body.slang";
__include "pragma-warning-include-pushpop-footer.slang";

// The body file's warning (after the disable) must stay suppressed, so no *second* E30856
// may appear after the positive control above.
// CHECK-NOT: E30856
