// A bodyless '[CudaKernel]' function must be rejected with a diagnostic, not crash the compiler,
// when compiled for the cuda/torch targets. See https://github.com/shader-slang/slang/issues/12515
//
// 'result code = -1' is asserted alongside the diagnostic because a SIMPLE test passes even if the
// compiler prints the error and then crashes; pinning the exit code catches that regression.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none
//TEST:SIMPLE(filecheck=CHECK): -target torch -line-directive-mode none
//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none -DTORCH_ENTRY
//TEST:SIMPLE(filecheck=CHECK): -target torch -line-directive-mode none -DTORCH_ENTRY
//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none -DAUTOBIND
//TEST:SIMPLE(filecheck=CHECK): -target torch -line-directive-mode none -DAUTOBIND
//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none -DDERIV
//TEST:SIMPLE(filecheck=CHECK): -target torch -line-directive-mode none -DDERIV

#if defined(DERIV)

// Reaches the derivative-wrapper path (generateDerivativeWrappers).
[AutoPyBindCUDA]
[CudaKernel]
[Differentiable]
void kernel(no_diff float x);

#elif defined(AUTOBIND)

// No registered derivatives: reaches the host-wrapper path (generateHostFunctionsForAutoBindCuda).
[AutoPyBindCUDA]
[CudaKernel]
void kernel(float x);

#elif defined(TORCH_ENTRY)

[TorchEntryPoint]
[CudaKernel]
void kernel(float x);

#else

[CudaKernel]
void kernel(float x);

#endif

// CHECK: result code = -1
// CHECK: error[E55104]
