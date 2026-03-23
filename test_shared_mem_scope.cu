// Use scoped shared memory via a union to force overlapping manually
extern "C" __global__ void kernel_manual_overlap(float* out)
{
    // Manual overlap: union forces both arrays to share the same memory
    __shared__ union {
        float smem1[16]; // 64 bytes
        float smem2[32]; // 128 bytes
    } shared_mem;

    int idx = threadIdx.x;

    // Phase 1
    shared_mem.smem1[idx] = float(idx);
    __syncthreads();
    out[idx] = shared_mem.smem1[idx];
    __syncthreads();

    // Phase 2
    shared_mem.smem2[idx] = float(idx) * 2.0f;
    __syncthreads();
    out[idx] += shared_mem.smem2[idx];
}
