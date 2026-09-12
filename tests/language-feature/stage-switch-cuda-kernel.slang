//TEST:SIMPLE(filecheck=CHECK):-target cuda -line-directive-mode none

// Test that __stage_switch resolves to the `compute` case when called
// from a [CUDAKernel] function. This is a regression test for
// https://github.com/shader-slang/slang/issues/9942

[ForceInline]
int getWaveCount()
{
    __stage_switch
    {
    case compute:
        __target_switch
        {
        case cuda:
            uint3 blockDim = cudaBlockDim();
            int warpsPerBlock = (blockDim.x * blockDim.y * blockDim.z) >> 5;
            return warpsPerBlock;
        }
    default:
        static_assert(false, "Only support compute stage");
        return 0;
    }
}

// getWaveCount is a user [ForceInline]; on CUDA it is deferred to NVRTC and emitted as a separate
// __forceinline__ __device__ function (#12623). __stage_switch still resolves to the compute case,
// so blockDim is present and the default static_assert ("Only support compute stage") is not taken.
// CHECK: __forceinline__ __device__ int getWaveCount
// CHECK: blockDim
// CHECK: __global__ void forward
// CHECK-NOT: Only support compute stage
[CUDAKernel]
void forward(TensorView<float> output)
{
    int waveCount = getWaveCount();
    output.store(cudaThreadIdx().x, float(waveCount));
}
