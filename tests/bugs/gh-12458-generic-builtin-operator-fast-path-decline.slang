// Companion negative case to `gh-12458-generic-builtin-operator-fast-path.slang`: the widened
// builtin-operator fast path must be DECLINED for a generic type parameter that has no proven
// conformance to any of the three sealed builtin marker interfaces
// (`SemanticsExprVisitor::classifyBuiltinArithmeticElementType` returns "unknown" for it), not
// just correctly taken when conformance is proven. Declining means falling through to ordinary
// `operator+` overload resolution, which finds no candidate for a fully abstract `T` and reports
// the same diagnostic it always has -- a regression that made the fast path fire unconditionally
// (e.g. treating "unknown" as "assume floating-point") would instead compile this cleanly or
// report a different error.
//
// This does not cover the fast path's OWN `isWitnessUncheckedOptional` guard (the case that
// distinguishes this PR from a naive version of the fix: a `where optional T :
// __BuiltinFloatingPointType` constraint referenced outside an `if (T is ...)` block must also be
// declined, because an unchecked optional witness is not proof of conformance). That guard is
// verified correct in isolation -- instrumenting `classifyBuiltinArithmeticElementType` confirms
// it recognizes the direct `__BuiltinFloatingPointType` witness as unchecked-optional and declines
// (`family.isFloat = false`) -- but the expression still compiles cleanly with no diagnostic,
// because it then falls through to ordinary overload resolution, which independently (and
// unconditionally of this PR's guard) accepts it: `__BuiltinFloatingPointType` transitively
// implies `IArithmetic` (`__BuiltinFloatingPointType : __BuiltinRealType, IFloat, ...` and `IFloat
// : IArithmetic`), so `a + b` matches a generic `IArithmetic`-constrained `operator+` overload in
// core.meta.slang without ever going through the unchecked-optional check that guards ordinary
// member lookup. That's a pre-existing, broader soundness gap in operator-expression overload
// resolution, unrelated to and not introduced by this PR (confirmed by reproducing the silent
// accept against `master` with none of this PR's changes applied) -- tracked as
// shader-slang/slang#13061, which also documents a concrete compiler crash
// (`InternalError: unexpected: Unexpected context type for parameter info retrieval`) when such a
// function is instantiated with a genuinely non-conforming type. A diagnostic test for this PR's
// own guard isn't reachable until #13061 is fixed: today, every attempt compiles clean (masking
// the guard) or crashes (which isn't a diagnostic to assert on). Add the test once #13061 lands.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):

T declineUnconstrainedT<T>(T a, T b) { return a + b; }
//CHECK: no overload for '+' applicable to arguments of type (T, T)
