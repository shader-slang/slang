// A bodyless '[TorchEntryPoint]' function must be rejected with a diagnostic, not crash the
// compiler, when targeting torch. See https://github.com/shader-slang/slang/issues/12512

//TEST:SIMPLE(filecheck=CHECK): -target torch -line-directive-mode none

[TorchEntryPoint]
float foo(float x);

// CHECK: result code = -1
// CHECK: error[E55103]: '[TorchEntryPoint]' function requires a body
