//TEST:SIMPLE(filecheck=CHECK):

module test_pragma_warning;

__include "pragma-warning-multifile-impl1.slang";
__include "pragma-warning-multifile-impl2.slang";

// CHECK-NOT: warning 30856