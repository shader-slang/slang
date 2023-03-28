//DISABLE_TEST:SIMPLE: -target cuda -line-directive-mode none

// Verify that we can output a cuda device function with [CudaDeviceExport].
// Disabled until we have FileCheck.

struct MySubType
{
    TorchTensor<float> array[2];
}

struct MyType
{
    float2 v;
    MySubType sub[2];
}

struct MyInput
{
    TorchTensor<float> inValues;
    float normalVal;
}

[CudaKernel]
void myKernel(TensorView<float> inValues, TensorView<float> outValues)
{
    if (cudaThreadIdx().x > 0)
        return;
    outValues.store(cudaThreadIdx().x, sin(inValues.load(cudaThreadIdx().x)));
}

[TorchEntryPoint]
public __extern_cpp MyType runCompute(MyInput input)
{
    MyType rs;
    var outValues = TorchTensor<float>.alloc(1);
    let inValues = input.inValues;
    
    __dispatch_kernel(myKernel, uint3(1, 1, 1), uint3(32, 1, 1))(inValues, outValues);

    rs.v = float2(1.0, 2.0);
    rs.sub[0].array[0] = outValues;
    rs.sub[0].array[1] = inValues;

    rs.sub[1].array[0] = inValues;
    rs.sub[1].array[1] = outValues;
    return rs;
}