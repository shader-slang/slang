//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Companion to generic-arithmetic-coerce.slang: pins the *rejected* side of
// the `_coerce` fast-rejection of opaque generic type parameters into scalar
// builtins.
//
// - `implicitPlain` locks in the diagnostic itself: implicit `T -> int` at a
//   scalar target with no equality witness in scope fails as a plain type
//   mismatch (E30019). A change to the shortcut's diagnostic (code, severity,
//   or which types it names) shows up here.
// - `implicitWithExtension` pins the rejection's documented known assumption:
//   no scalar builtin declares an initializer whose parameter type is an
//   opaque generic parameter. The file-scope
//   `extension int { __init<U : IArithmetic>(U) }` below violates that
//   assumption — before the fast rejection existed, the initializer search
//   would have matched it (U = T, an exact match even at an implicit site) —
//   but the rejection fires first and never sees it, so the coercion fails
//   identically to the plain case. If this shape ever needs supporting, the
//   rejection must instead be gated on a cached per-`toType` "has a generic
//   initializer" check, and this case flips from error to success.
//
// Note the extension applies file-wide, so `implicitPlain` is nominally
// subject to it too; that is exactly the point — with the fast rejection in
// place the extension changes nothing, and both sites emit the same error.

int implicitPlain<T : IArithmetic>(T t)
{
    int x = t;
//CHECK:    ^ error E30019
//CHECK:    ^ expected an expression of type 'int', got 'T'
    return x;
}

extension int
{
    // `__implicit_conversion` is what would have made this initializer viable
    // at an implicit site pre-rejection; a plain `__init` only supports
    // explicit conversion and would never have matched anyway.
    __implicit_conversion(100)
    __init<U : IArithmetic>(U u)
    {
        this = 0;
    }
}

int implicitWithExtension<T : IArithmetic>(T t)
{
    int y = t;
//CHECK:    ^ error E30019
//CHECK:    ^ expected an expression of type 'int', got 'T'
    return y;
}
