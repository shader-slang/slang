//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Companion to generic-arithmetic-coerce.slang: pins the target-type boundary
// of the `_coerce` fast-rejection of opaque generic type parameters into
// scalar builtins. `BFloat16` (like `FloatE4M3` / `FloatE5M2`) is a scalar
// builtin to the user, but it is a `DeclRefType`, not a `BasicExpressionType`,
// and it declares a generic initializer
// (`__init<U : IFloatingPointCoopElement>(U)`), so a coercion toward it must
// still take the initializer search.
//
// That initializer is not `__implicit_conversion`-marked, so the implicit
// coercion below fails either way — what the search buys at an implicit site
// is the "explicit conversion from 'T' to 'BFloat16' is possible" note, which
// is only emitted after the search actually ran and found the explicit-capable
// candidate. A future "unification" that broadens the rejection's target test
// from `BasicExpressionType` to any scalar-looking builtin would skip the
// search and silently drop that note; this test pins it.

BFloat16 convert<T : IFloatingPointCoopElement>(T t)
{
    BFloat16 b = t;
//CHECK:         ^ error E30019
//CHECK:         ^ expected an expression of type 'BFloat16', got 'T'
//CHECK:         ^ explicit conversion from 'T' to 'BFloat16' is possible
    return b;
}
