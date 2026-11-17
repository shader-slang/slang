// A bodyless '[TorchEntryPoint]' function must be rejected with a diagnostic, not crash the
// compiler, when targeting torch. See https://github.com/shader-slang/slang/issues/12512

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -target torch -line-directive-mode none

[TorchEntryPoint]
float foo(float x);

// CHECK: error[E55103]: '[TorchEntryPoint]' function requires a body
