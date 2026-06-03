//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_CPP,non-exhaustive): -target cpp -stage node -entry main -experimental-feature -DCPP_TARGET
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_CUDA,non-exhaustive): -target cuda -stage node -entry main -experimental-feature -DCUDA_TARGET
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_GLSL,non-exhaustive): -target glsl -stage node -entry main -experimental-feature -DGLSL_TARGET
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_METAL,non-exhaustive): -target metal -stage node -entry main -experimental-feature -DMETAL_TARGET
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_WGSL,non-exhaustive): -target wgsl -stage node -entry main -experimental-feature -DWGSL_TARGET
import experimental.workgraph;

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main()
{
}

#if defined(CPP_TARGET)
//CHECK_CPP: intrinsic operation 'C++ node shader stage' is not supported for the current target.
#elif defined(CUDA_TARGET)
//CHECK_CUDA: intrinsic operation 'CUDA node shader stage' is not supported for the current target.
#elif defined(GLSL_TARGET)
//CHECK_GLSL: intrinsic operation 'GLSL node shader stage' is not supported for the current target.
#elif defined(METAL_TARGET)
//CHECK_METAL: intrinsic operation 'Metal node shader stage' is not supported for the current target.
#elif defined(WGSL_TARGET)
//CHECK_WGSL: intrinsic operation 'WGSL node shader stage' is not supported for the current target.
#endif
