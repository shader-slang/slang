//TEST:SIMPLE(filecheck=CHECK):

module nested_pragma_test;

#pragma warning(disable: 30856)

__include "nested-pragma-impl1.slang";

// The root-level disable suppresses 30856 across the nested `__include`s (no push/pop here),
// so it must not appear. (The diagnostic renders as `warning[E30856]`; match on the `E30856`
// code so this check is not vacuous.)
// CHECK-NOT: E30856