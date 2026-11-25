//TEST:COMPILE: tests/autodiff/transitive-attribute-extensions-serialized-lib.slang -o tests/autodiff/transitive-attribute-extensions-serialized-lib.slang-module
//TEST:COMPARE_COMPUTE(targets=cuda+cpu, filecheck-buffer=CHECK):-output-using-type

// The consumer imports only a facade whose library loads from the precompiled module above, so
// the provenance of attribute-generated extensions must survive AST serialization.
#define SERIALIZED_LIBRARY 1
#include "transitive-attribute-extensions.slang"
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
// CHECK: 6.0
// CHECK: 6.0
// CHECK: 6.0
// CHECK: 6.0
// CHECK: 9.0
// CHECK: 15.0
// CHECK: 2.0
// CHECK: 21.0
// CHECK: 33.0
