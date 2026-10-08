//TEST:SIMPLE(filecheck=CHECK):
// #ifdef support

// CHECK-NOT: warning[E30081]
#pragma warning (pop)

// Test that #pragma warning (pop) emits a warning when the stack is empty