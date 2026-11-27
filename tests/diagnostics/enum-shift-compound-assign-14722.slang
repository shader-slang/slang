//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=2):-target spirv

// Pin today's boundary of the enum-as-shift-count fix (shader-slang/slang#14722):
// the compound form `x <<= SomeEnum` still finds no overload, because
// `operator<<=` is declared only as the fully generic
// `__generic<L, R : __BuiltinIntegerType>` in the `kShiftOps` section of
// core.meta.slang, and a generic's type argument must conform before
// argument coercion can apply the implicit enum-to-tag conversion. The
// E40026 note now fires alongside the error — `x << SomeEnum` type-checks
// through the concrete overloads, so the compiler can offer the
// `x = x << y` rewrite — which is also why this diagnostic is pinned here:
// the note only exists when the binary form applies.
// The compound gap itself is tracked as shader-slang/slang#14727; once that
// is fixed this test is expected to succeed, and the pins below should be
// replaced by coverage of the then-working compound form.

enum Options { Shift = 6 };

int shiftAssign(int x)
{
    x <<= Options.Shift;
    return x;
}

// CHECK: error[E39999]: no overload for '<<=' applicable to arguments of type (int, Options)
// CHECK: note[E40026]: '<<' applies to these arguments, but '<<=' is not derived from it: declare 'operator<<=' with an 'inout' first parameter, or write 'x = x << y'
