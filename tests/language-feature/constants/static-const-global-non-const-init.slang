//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl

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

// A pure callable (e.g. min) with a runtime argument must also be diagnosed.
// This exercises the isPure-with-runtime-arg recursion path in _initExprIsRuntimeValue.
// CHECK: error[E31226]
static const int minVal = min(int(foo), 5);
//CHECK:         ^^^^^^ static const global initializer must be a compile-time constant
//CHECK:         ^^^^^^ initializer of static const global 'minVal' does not evaluate to a compile-time constant

// A specialization constant in one element does not stop E31226 for a mutable global in
// another element of the same initializer.
[vk::constant_id(0)]
const int specN = 4;
// CHECK: error[E31226]
static const int mixed[2] = { specN, int(foo) };
//CHECK:         ^^^^^ static const global initializer must be a compile-time constant
//CHECK:         ^^^^^ initializer of static const global 'mixed' does not evaluate to a compile-time constant

// The exemption covers only specialization constants that are shader parameters. A
// `groupshared` global that carries the attribute is still mutable workgroup storage.
[vk::constant_id(1)]
groupshared int sharedSpec;
// CHECK: error[E31226]
static const int fromShared[2] = { sharedSpec, 3 };
//CHECK:         ^^^^^^^^^^ static const global initializer must be a compile-time constant
//CHECK:         ^^^^^^^^^^ initializer of static const global 'fromShared' does not evaluate to a compile-time constant
