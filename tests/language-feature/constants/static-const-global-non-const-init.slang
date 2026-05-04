//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// Regression test for: https://github.com/shader-slang/slang/issues/8550
// A `static const` global initialized from a non-const global should produce
// a diagnostic error instead of crashing the compiler.

static float foo = 1.0f;
static const int bar = int(foo * 25);
//CHECK:         ^^^ static const global initializer must be a compile-time constant
//CHECK:         ^^^ initializer of static const global 'bar' does not evaluate to a compile-time constant

// The diagnostic also fires for non-integer types.
static const float baz = foo * 2.0f;
//CHECK:           ^^^ static const global initializer must be a compile-time constant
//CHECK:           ^^^ initializer of static const global 'baz' does not evaluate to a compile-time constant
