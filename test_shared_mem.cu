__device__ void function1(float* out, int idx)
{
    __shared__ float smem1[16]; // 16 * 4 = 64 bytes
    smem1[idx] = float(idx);
    __syncthreads();
    out[idx] = smem1[idx];
}

__device__ void function2(float* out, int idx)
{
    __shared__ float smem2[32]; // 32 * 4 = 128 bytes
    smem2[idx] = float(idx) * 2.0f;
    __syncthreads();
    out[idx] += smem2[idx];
}

extern "C" __global__ void kernel(float* out)
{
    int idx = threadIdx.x;
    function1(out, idx);
    function2(out, idx);
}
