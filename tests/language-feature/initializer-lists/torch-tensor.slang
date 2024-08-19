//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none
//TEST:SIMPLE(filecheck=TORCH): -target torch -line-directive-mode none


struct MySubType
{
    TorchTensor<float> array[2];
}

struct MyType
{
    float2 v;
    MySubType sub[2];
}

struct MyType2
{
    float2 v;
    TorchTensor<float> tensor;
}

struct ReturnType
{
    MyType t1;
    MyType2 t2;
}

struct MyInput
{
    TorchTensor<float> inValues;
    float normalVal;
}

// CUDA: __global__ void myKernel(TensorView inValues_[[#]], TensorView outValues_[[#]])
[CudaKernel]
void myKernel(TensorView<float> inValues, TensorView<float> outValues)
{
    if (cudaThreadIdx().x > 0)
        return;
    outValues.store(cudaThreadIdx().x, sin(inValues.load(cudaThreadIdx().x)));
}

// TORCH:      {{^SLANG_PRELUDE_EXPORT$}}
// TORCH-NEXT: void myKernel(TensorView {{[[:alnum:]_]+}}, TensorView {{[[:alnum:]_]+}});
//
// TORCH:      {{^SLANG_PRELUDE_EXPORT$}}
// TORCH-NEXT: runCompute(std::tuple<torch::Tensor, float> input_[[#]])
[TorchEntryPoint]
export __extern_cpp ReturnType runCompute(MyInput input)
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

    MyType2 rs2;
    rs2.tensor = outValues;

    ReturnType returnVal;
    returnVal.t1 = rs;
    returnVal.t2 = rs2;
    return returnVal;
}
