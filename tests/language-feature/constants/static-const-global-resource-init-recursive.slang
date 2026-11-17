//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):-target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):-target hlsl

// A struct that contains itself by value is invalid. Using a shader parameter of such a
// type to initialize a `static const` global must produce diagnostics, not crash the
// compiler while checking whether the type is made only of resources
// (https://github.com/shader-slang/slang/issues/14378).

struct R
{
    Texture2D t;
    R r;
}
R rp;

static const R x = rp;
//CHECK:       ^ static const global initializer must be a compile-time constant
