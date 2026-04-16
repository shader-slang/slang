// Regression test for late-specialized `[ForceUnroll]` loops reaching autodiff
// before loop unrolling has run.
//
//TEST:SIMPLE(filecheck=CHECK): -target cuda -line-directive-mode none

// CHECK-LABEL: __device__ float3  loadVecOnce_0(
// CHECK: *&((&result_0)->x) =
// CHECK: *&((&result_0)->y) =
// CHECK: *&((&result_0)->z) =
//
// CHECK: __global__ void __kernel__loss_kernel_loadvec_sum_bwd_diff(uint {{[[:alnum:]_]+}}, float {{[[:alnum:]_]+}}, DiffTensorView_[[#]] {{[[:alnum:]_]+}}, DiffTensorView_[[#]] {{[[:alnum:]_]+}})
// CHECK: __global__ void __kernel__loss_kernel_loadvec_sum_fwd_diff(uint {{[[:alnum:]_]+}}, float {{[[:alnum:]_]+}}, DiffTensorView_[[#]] {{[[:alnum:]_]+}}, DiffTensorView_[[#]] {{[[:alnum:]_]+}})
// CHECK: __global__ void __kernel__loss_kernel_loadvec_sum(uint {{[[:alnum:]_]+}}, float {{[[:alnum:]_]+}}, DiffTensorView_[[#]] {{[[:alnum:]_]+}}, DiffTensorView_[[#]] {{[[:alnum:]_]+}})

__generic<T : __BuiltinFloatingPointType, A : IDiffTensorWrapper>
extension DiffTensorView<T, A>
{
    [Differentiable]
    __generic<let M : int, let R : int, let N : int>
    vector<T, M> loadVecOnce(vector<uint, N> x)
    {
        vector<T, M> result;
        [ForceUnroll]
        for (int j = 0; j < M; j++)
        {
            result[j] = this.loadOnce(x);
            x[R] += 1;
        }
        return result;
    }

    [Differentiable]
    __generic<let M : int, let N : int>
    vector<T, M> loadVecOnce(vector<uint, N> x)
    {
        return this.loadVecOnce<M, N - 1, N>(x);
    }
}

[CudaKernel]
[Differentiable]
[AutoPyBindCUDA]
void loss_kernel_loadvec_sum(
    no_diff uint count,
    no_diff float invCount,
    DiffTensorView<float> positions,
    DiffTensorView<float> lossOut)
{
    uint idx = cudaThreadIdx().x + cudaBlockIdx().x * cudaBlockDim().x;
    if (idx >= count)
        return;

    float3 pos = positions.loadVecOnce<3>(uint2(idx, 0));
    lossOut.storeOnce(idx, (pos.x + pos.y + pos.z) * invCount);
}
