//TEST:SIMPLE(filecheck=CUDA): -O0 -target cuda -validate-ir -line-directive-mode none

// `__dispatch_kernel` launches the original CUDA kernel rather than calling its ordinary-function
// clone. Resource-global legalization gives the original kernel a local whose initializer runs for
// each launch, so the dispatch needs no generated resource argument.

Texture2D<float4> inputTexture;
RWStructuredBuffer<float4> output;
static Texture2D<float4> cachedTexture = inputTexture;

[CudaKernel]
void readCachedTexture()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0));
}

// We export the host wrapper so that linking retains the dispatch until resource-global
// legalization classifies its use of `readCachedTexture`.
[CudaHost]
export void launchKernel()
{
    __dispatch_kernel(readCachedTexture, uint3(1, 1, 1), uint3(1, 1, 1))();
}

// CUDA-LABEL: __global__ void readCachedTexture()
// CUDA: tex2Dfetch
// CUDA-LABEL: __host__ void launchKernel()
// CUDA: readCachedTexture<<<
