// Regression test for #11297: using the `String` type (and its member
// operations such as `getLength`) when generating kernel C++ code produced
// uncompilable output referencing an undefined `String` type/method, with no
// diagnostic. The `String` type has no runtime representation in kernel
// C++/CUDA output, so its use must be diagnosed (E55213) at the use site.
//
// `NativeString` (which lowers to `const char*`) remains the supported way to
// work with strings on these targets.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target cpp -entry computeMain -stage compute

RWStructuredBuffer<int64_t> output;

void computeMain()
{
    let str : String = "123";
    output[0] = str.getLength();
//CHECK:                     ^ E55213
}
