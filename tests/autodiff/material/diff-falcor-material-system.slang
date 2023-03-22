//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

import MaterialSystem;
import DiffuseMaterial;
import DiffuseMaterialInstance;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    IMaterial material = createMaterialClassConformance(0, float3(0.9, 0.6, 0.2));
    let mi = material.setupMaterialInstance();

    float3 wi = normalize(float3(0.5, 0.2, 0.8));
    float3 wo = normalize(float3(-0.1, -0.3, 0.9));
    float3 f = mi.eval(wi, wo);

    outputBuffer[0] = f[0];
    outputBuffer[1] = f[1];
    outputBuffer[2] = f[2];
}
