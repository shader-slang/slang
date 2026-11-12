// Minimal repro for GitHub #9267: max() autodiff gradient depends on load pattern.
// Run with run_9267_reveal.py (slangtorch + PyTorch). Reveals bug when gradient
// from loadVecOnce path != gradient from manual load path.
//
// Kernels: loss_kernel_vec_max (loadVecOnce<3> + max) vs loss_kernel_vec_max_manual
// (manual loadOnce x3 + same max). Same math; gradients must match.

__generic<T : __BuiltinFloatingPointType>
extension TensorView<T>
{
    __generic<let M : int, let R : int, let N : int>
    vector<T, M> loadVec(vector<uint, N> x)
    {
        vector<T, M> result;
        [ForceUnroll]
        for (int j = 0; j < M; j++)
        {
            result[j] = this.load(x);
            x[R] += 1;
        }
        return result;
    }
    __generic<let M : int, let N : int>
    vector<T, M> loadVec(vector<uint, N> x)
    {
        return this.loadVec<M, N - 1, N>(x);
    }
}

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

[CUDAKernel]
[Differentiable]
[AutoPyBindCUDA]
void loss_kernel_vec_max(
    no_diff uint count,
    no_diff float invCount,
    DiffTensorView<float> positions,
    no_diff TensorView<float> dims,
    DiffTensorView<float> lossOut)
{
    uint idx = cudaThreadIdx().x + cudaBlockIdx().x * cudaBlockDim().x;
    if (idx >= count) return;
    float3 pos = positions.loadVecOnce<3>(uint2(idx, 0));
    float3 half = no_diff(dims.loadVec<3>(uint2(idx, 0))) * 0.5f;
    float3 exceed = abs(pos) - half;
    float3 relu = max(exceed, float3(0.0f));
    float loss_value = (relu.x + relu.y + relu.z) * invCount;
    lossOut.storeOnce(idx, loss_value);
}

[CUDAKernel]
[Differentiable]
[AutoPyBindCUDA]
void loss_kernel_vec_max_manual(
    no_diff uint count,
    no_diff float invCount,
    DiffTensorView<float> positions,
    no_diff TensorView<float> dims,
    DiffTensorView<float> lossOut)
{
    uint idx = cudaThreadIdx().x + cudaBlockIdx().x * cudaBlockDim().x;
    if (idx >= count) return;
    float3 pos;
    pos.x = positions.loadOnce(uint2(idx, 0));
    pos.y = positions.loadOnce(uint2(idx, 1));
    pos.z = positions.loadOnce(uint2(idx, 2));
    float3 half;
    half.x = dims.load(uint2(idx, 0));
    half.y = dims.load(uint2(idx, 1));
    half.z = dims.load(uint2(idx, 2));
    half = half * 0.5f;
    float3 exceed = abs(pos) - half;
    float3 relu = max(exceed, float3(0.0f));
    float loss_value = (relu.x + relu.y + relu.z) * invCount;
    lossOut.storeOnce(idx, loss_value);
}
