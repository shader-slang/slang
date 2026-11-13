//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -E

// The disabled line must be lexed after the conditional suppresses diagnostics.
#if 0
"disabled
#endif

// Consuming the endif delimiter must restore diagnostics before this line is lexed.
"enabled

//CHECK-NOT: disabled
//CHECK: newline in literal
//CHECK: "enabled
//CHECK-NOT: newline in literal
