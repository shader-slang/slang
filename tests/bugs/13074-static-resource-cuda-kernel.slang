//TEST:SIMPLE: -target cuda -o 13074-static-resource-cuda-kernel.cuda.cpp

Texture2D<float4> inputTexture;
RWStructuredBuffer<float4> output;

static Texture2D<float4> cachedTexture = inputTexture;

[CudaKernel]
void kernelMain()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0));
}
