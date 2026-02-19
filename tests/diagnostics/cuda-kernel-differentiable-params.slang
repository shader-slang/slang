//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Simple check to see if the compiler throws an error if a CUDA kernel is declared with non-void return type.

[CudaKernel]
[Differentiable]
void myBadKernel(float x, TensorView<float> t1, TensorView<float> t2)
//diag:                ^ differentiable kernel param restriction
//diag:                ^ differentiable kernel entry point cannot have differentiable parameters.
{
}

[CudaKernel]
void myGoodKernel(float x, TensorView<float> t1, TensorView<float> t2)
{

}
