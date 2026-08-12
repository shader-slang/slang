// A bodyless '[TorchEntryPoint]' function (a forward declaration) used to SIGSEGV the compiler when
// targeting torch: the PyTorch C++-binding pass dereferenced the function's null first block.
// See https://github.com/shader-slang/slang/issues/12512
//
// It must now produce a clean diagnostic (E55103) and a non-zero result code instead of crashing.
// The E55103 check is the real discriminator: before the fix the compiler crashed before emitting
// any diagnostic, so seeing this error code proves the guard ran. The `result code = -1` check
// additionally pins that the compile reports failure rather than wrongly succeeding.

//TEST:SIMPLE(filecheck=CHECK): -target torch -line-directive-mode none

[TorchEntryPoint]
float foo(float x);

// CHECK: result code = -1
// CHECK: error{{.*}}55103
// CHECK: requires a body
