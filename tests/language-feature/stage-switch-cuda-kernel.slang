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

// CHECK: __global__ void forward
// CHECK: blockDim
// CHECK-NOT: Only support compute stage
[CUDAKernel]
void forward(TensorView<float> output)
{
    int waveCount = getWaveCount();
    output.store(cudaThreadIdx().x, float(waveCount));
}
