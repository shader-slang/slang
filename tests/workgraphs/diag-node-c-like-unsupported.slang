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
//CHECK_CPP: entrypoint 'main' uses features that are not available in '_node' stage for 'cpp' compilation target.
#elif defined(CUDA_TARGET)
//CHECK_CUDA: entrypoint 'main' uses features that are not available in '_node' stage for 'cuda' compilation target.
#elif defined(GLSL_TARGET)
//CHECK_GLSL: entrypoint 'main' uses features that are not available in '_node' stage for 'glsl' compilation target.
#elif defined(METAL_TARGET)
//CHECK_METAL: entrypoint 'main' uses features that are not available in '_node' stage for 'metal' compilation target.
#elif defined(WGSL_TARGET)
//CHECK_WGSL: entrypoint 'main' uses features that are not available in '_node' stage for 'wgsl' compilation target.
#endif
