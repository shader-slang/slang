//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -vk -compute -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -dx12 -compute -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -mtl -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=gOutput
RWStructuredBuffer<int> gOutput;

struct DomeLight
{
    DescriptorHandle<Texture2D<float4>> hdrTexture;
    DescriptorHandle<SamplerState> linearSampler;

    int eval(int x) { return x + 1; }
}

Optional<DomeLight> getDomeLight()
{
    return none;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    let light = getDomeLight();
    if (light.hasValue == false)
        gOutput[tid] = 1;
    else
        gOutput[tid] = 0;
}

// CHECK: 1
