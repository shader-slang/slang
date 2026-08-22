//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -no-codegen

// The `template< ... >` declaration form is recognized only in the
// HLSL-flavored dialect. In ordinary Slang code `template` is a plain
// identifier, so `template<typename T>` must NOT be parsed as a template
// declaration; the parser reads `template` as an identifier and then reports an
// error on the following `<`. This locks in that template syntax does not leak
// into the Slang language.

template<typename T> struct W { T v; }
//CHECK: '>' expected
