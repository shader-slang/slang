//TEST:COMPILE: tests/autodiff/transitive-attribute-extensions-lib.slang -o tests/autodiff/transitive-attribute-extensions-lib.slang-module
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

// Import prefers the compiled library. Attribute provenance must survive AST
// serialization, even though the consumer imports only the facade module.
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
