//TEST:SIMPLE(filecheck=CHECK):

// Regression test for shader-slang/slang#8166. Part of the cross-`__include`
// `#pragma warning` family alongside `pragma-warning-multifile-*`,
// `pragma-warning-include-pushpop-*`, and `nested-pragma-*`.
//
// #8166: two separate `__include`d files each carry their own self-contained
// `#pragma warning(push)/(disable)/(pop)` cluster around code that would otherwise
// warn (mirroring slangpy's `atomics.slang` and `tensor.slang`). The *second* file's
// directives were spuriously rejected with `warning[E15615] cannot insert pragma here`,
// because the shared warning timeline keyed entries by an absolute source location whose
// axis restarted per preprocessing pass and so ran backwards on the later-but-lower file.
//
// Correct behaviour: both files' clusters suppress E30856 for their own code, no E15615
// is emitted, and the suppression does not leak past either file's `pop`.

module pragma_percluster_test;

__include "pragma-warning-include-percluster-atomics.slang";
__include "pragma-warning-include-percluster-tensor.slang";

namespace control
{
    interface IConvertibleFrom<From> {}

    // Positive control AFTER both includes (and after both files have popped their disables):
    // this extension MUST emit E30856, proving the included clusters' disables were correctly
    // scoped and did not leak here, and guarding the `CHECK-NOT` below from passing vacuously.
    extension<From : __BuiltinIntegerType, To : IConvertibleFrom<From>, let N : int>
    vector<To, N> : IConvertibleFrom<vector<From, N>> {}
}
// CHECK: warning[E30856]
// CHECK-NEXT: pragma-warning-include-percluster-main.slang

// The spurious "cannot insert pragma here" diagnostic from #8166 must never appear, for either
// included file's directives.
// CHECK-NOT: E15615
