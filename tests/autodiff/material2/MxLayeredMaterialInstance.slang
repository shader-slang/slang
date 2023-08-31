//TEST_IGNORE_FILE:

__exported import IMaterialInstance;
__exported import IBSDF;

import MxWeights;

struct MxLayeredMaterialInstance<
    let TBsdfCount : int,
    let TLayerCount : int,
    TLayeredMaterialData : IMxLayeredMaterialData,
    TWeightsCalc : IMxLayeredWeightCalculator> : IMaterialInstance
{
    TLayeredMaterialData data;
    TWeightsCalc calculator;
    typealias Weights = MxWeights<TBsdfCount>;

    Weights calculateWeights()
    {
        Weights result;
        calculator.calculateWeights<TBsdfCount, TLayerCount, TLayeredMaterialData>(data, result);
        return result;
    }

    [Differentiable]
    float3 eval(const MaterialInstanceData miData, const float3 wi, const float3 wo)
    {
        float3 albedo;
        for (uint i = 0; i < 3; i++) albedo[i] = miData.data[i];

        float3 result = float3(1.f);
        [ForceUnroll] for (uint i = 0; i < TBsdfCount; i++) result *= albedo;
        return result;
    }
}
