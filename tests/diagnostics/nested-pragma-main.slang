//TEST:SIMPLE(filecheck=CHECK):

module nested_pragma_test;

#pragma warning(disable: 30856)

__include "nested-pragma-impl1.slang";

// CHECK-NOT: warning 30856