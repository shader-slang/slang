//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -no-codegen

// A template's *inner* declaration is validated against the enclosing
// container, so wrapping a declaration in a template does not slip it past its
// nesting rule: a `namespace` is not allowed inside a `struct`, and that is
// reported on the inner declaration.

struct Outer
{
    template<typename T>
    namespace N
    {
    }
};
//CHECK: namespace is not allowed here
