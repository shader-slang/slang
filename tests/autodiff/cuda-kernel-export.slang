//DISABLE_TEST:SIMPLE: -target cuda -line-directive-mode none

// Verify that we can output a cuda device function with [CudaDeviceExport].
// Disabled until we have FileCheck.


[CudaKernel]
void myKernel(TensorView<float> inValues, TensorView<float> outValues)
{
    if (cudaThreadIdx().x > 0)
        return;
    outValues.store(cudaThreadIdx().x, sin(inValues.load(cudaThreadIdx().x)));
}

[TorchEntryPoint]
public __extern_cpp TorchTensor<float> runCompute(TorchTensor<float> inValues)
{
    var outValues = TorchTensor<float>.alloc(1);
    __dispatch_kernel(myKernel, uint3(1, 1, 1), uint3(32, 1, 1))(inValues, outValues);
    return outValues;
}