//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

#define DIRECT_IMPORT 1
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
// CHECK: 4.0
