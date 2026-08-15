//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -no-codegen

// A template whose inner declaration fails to parse must recover gracefully
// rather than crash. Here the inner is a multi-declarator (`int a, b;`), for
// which `ParseSingleDecl` yields no single declaration; the leading modifier
// (`static`) then drives modifier attachment onto that inner. Both rely on the
// template having a non-null `inner`, which the parser guarantees by
// synthesizing an empty inner declaration in this case, so the module emits a
// single diagnostic instead of faulting.

static template<typename T> int a, b;
//CHECK: didn't expect multiple declarations here
