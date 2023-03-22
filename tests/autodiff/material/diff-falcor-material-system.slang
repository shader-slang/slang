//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

import MaterialSystem;
import DiffuseMaterial;
import DiffuseMaterialInstance;
import GlossyMaterial;
import GlossyMaterialInstance;

//TEST_INPUT: type_conformance DiffuseMaterial:IMaterial = 0
//TEST_INPUT: type_conformance GlossyMaterial:IMaterial = 1

[BackwardDifferentiable]
float3 evalBSDF(int type)
{
    float3 wi = normalize(float3(0.5, 0.2, 0.8));
    float3 wo = normalize(float3(-0.1, -0.3, 0.9));

    IMaterial material = createMaterialClassConformance(type, float3(0.9, 0.6, 0.2));
    let mi = material.setupMaterialInstance();
    float3 f = mi.eval(wi, wo);
    return f;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    /*
    {
        float3 f = evalBSDF(0);
        
        outputBuffer[0] = f[0];
        outputBuffer[1] = f[1];
        outputBuffer[2] = f[2];
    }
    */

    {
        DifferentialPair<float3> dp_f = __fwd_diff(evalBSDF)(0);

        outputBuffer[0] = dp_f.d[0];
        outputBuffer[1] = dp_f.d[1];
        outputBuffer[2] = dp_f.d[2];
    }

    /*
    {
        float3 f = evalBSDF(1);

        outputBuffer[3] = f[0];
        outputBuffer[4] = f[1];
        outputBuffer[5] = f[2];
    }
    */

    {
        DifferentialPair<float3> dp_f = __fwd_diff(evalBSDF)(1);

        outputBuffer[3] = dp_f.d[0];
        outputBuffer[4] = dp_f.d[1];
        outputBuffer[5] = dp_f.d[2];
    }
}
