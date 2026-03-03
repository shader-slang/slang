//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Simple check to see if the compiler throws an error if a CUDA kernel is declared with non-void return type.

[CudaKernel]
float myBadKernel(TensorView<float> t1, TensorView<float> t2)
/*diag:
      ^^^^^^^^^^^ CUDA kernel return type error
      ^^^^^^^^^^^ return type of a CUDA kernel function cannot be non-void.
*/
{
}

[CudaKernel]
void myGoodKernel(TensorView<float> t1, TensorView<float> t2)
{

}
