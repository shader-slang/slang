//TEST:SIMPLE(filecheck=CHECK): -target torch -line-directive-mode none
//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none

// The `-target torch` binding and the `-target cuda` kernels are compiled
// separately and pass `__extern_cpp` structs between them, so both spell the
// fields of such a struct by their source names
// (https://github.com/shader-slang/slang/issues/14028).

//CHECK: struct Params
//CHECK-NEXT: {
//CHECK-NEXT: float scale;
//CHECK-NEXT: int{{(32_t)?}} count;
//CHECK-NEXT: };

public __extern_cpp struct Params
{
    public float scale;
    public int count;
}

[CudaKernel]
void scaleKernel(TensorView<float> output, Params params)
{
    output[cudaThreadIdx().x] = params.scale * params.count;
}

[TorchEntryPoint]
public TorchTensor<float> run(Params params)
{
    var result = TorchTensor<float>.alloc(4);
    __dispatch_kernel(scaleKernel, uint3(1, 1, 1), uint3(4, 1, 1))(result, params);
    return result;
}
