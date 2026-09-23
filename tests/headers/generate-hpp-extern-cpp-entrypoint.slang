//TEST:SIMPLE(filecheck=CHECK): -target hpp -line-directive-mode none

// Regression test for https://github.com/shader-slang/slang/issues/9403 :
// an `__extern_cpp` compute entry point survives DCE and forces the C++ header
// path to emit its three CPU wrappers. In header mode these must be prototypes
// (declarations ending in `;`), not full definitions. Emitting the bodies made
// the header call the `_`-prefixed workhorse `_example`, which is never declared
// or defined in the header, so the header failed to compile.

//CHECK-NOT: _example
//CHECK: void example_Thread({{.*}});
//CHECK-NOT: _example
//CHECK: void example_Group({{.*}});
//CHECK-NOT: _example
//CHECK: void example({{.*}});
//CHECK-NOT: _example

[shader("compute")]
__extern_cpp void example() {}
