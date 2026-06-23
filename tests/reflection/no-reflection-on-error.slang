// Regression test for GitHub issue #11683, where the -reflection-json option
// segfaulted Slang if the input failed to compile.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -reflection-json no-reflection-on-error.json -target spirv

[shader("compute")]
void main() { foo } // forces a compile error

// CHECK: error
// CHECK-NOT: internal error