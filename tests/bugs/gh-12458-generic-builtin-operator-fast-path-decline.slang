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
// This test covers the absence of a conformance witness. An unchecked optional witness is a
// separate case: the builtin fast path must decline it, and ordinary overload resolution must
// reject the `IArithmetic` proof it inherits. That case is covered by
// `tests/language-feature/generics/where-optional-inherited-requirement*.slang`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):

T declineUnconstrainedT<T>(T a, T b) { return a + b; }
//CHECK: no overload for '+' applicable to arguments of type (T, T)
