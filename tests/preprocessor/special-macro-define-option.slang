//TEST:SIMPLE(filecheck=CHECK): -E -D__LINE__=100

// A command-line define shadows the built-in `__LINE__` at user level, and an
// `#undef` in the file restores the built-in.

__LINE__

#undef __LINE__

__LINE__

// CHECK: standard output = {
// CHECK: 100
// CHECK: 10
