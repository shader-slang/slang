// Version where the two shared arrays could potentially be overlapped
// since function1 completes (with sync) before function2 starts.

__device__ __noinline__ void function1(float* out, int idx)
{
    __shared__ float smem1[16]; // 64 bytes
    smem1[idx] = float(idx);
    __syncthreads();
    out[idx] = smem1[idx];
    __syncthreads(); // ensure all reads done before returning
}

__device__ __noinline__ void function2(float* out, int idx)
{
    __shared__ float smem2[32]; // 128 bytes
    smem2[idx] = float(idx) * 2.0f;
    __syncthreads();
    out[idx] += smem2[idx];
    __syncthreads();
}

extern "C" __global__ void kernel(float* out)
{
    int idx = threadIdx.x;
    function1(out, idx);
    // smem1 is no longer needed here
    function2(out, idx);
}
