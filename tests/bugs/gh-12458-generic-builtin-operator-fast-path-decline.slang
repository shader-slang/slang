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
// (`family.isFloat = false`). The expression then falls through to ordinary overload
// resolution, which rejects it as well: `__BuiltinFloatingPointType` only inherits `IArithmetic`,
// and the optional-constraint check also covers conformances an optional constraint inherits
// (shader-slang/slang#13061). That diagnostic, and the crash it replaces for a non-conforming type
// (shader-slang/slang#13011), are tested in
// `tests/language-feature/generics/where-optional-inherited-requirement*.slang`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):

T declineUnconstrainedT<T>(T a, T b) { return a + b; }
//CHECK: no overload for '+' applicable to arguments of type (T, T)
