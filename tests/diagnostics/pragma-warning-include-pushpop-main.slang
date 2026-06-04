//TEST:SIMPLE(filecheck=CHECK):

// Regression test for shader-slang/slang#11473.
// A root-level `#pragma warning(disable)` must keep suppressing the warning in an
// earlier `__include`d file even when a *later* `__include`d file performs an
// unrelated `#pragma warning(push)/(pop)`. The push/pop in the later file must not
// re-enable the root-level disable for the earlier file's code.

module pragma_pushpop_test;

#pragma warning(disable: 30856)

__include "pragma-warning-include-pushpop-body.slang";
__include "pragma-warning-include-pushpop-footer.slang";

// CHECK-NOT: E30856
