//TEST_IGNORE_FILE:

__exported import IMaterial;
__exported import IMaterialInstance;
import MxLayeredMaterialInstance;
import MxWeights;

struct LayeredData_mixedLobes : IMxLayeredMaterialData
{
    static const int bsdfCount = 3;
    static const int layerCount = 1;

    int getBsdfCount() { return bsdfCount; }
    int getLayerCount() { return layerCount; }
}
#define TMxLayeredMaterialData LayeredData_mixedLobes

struct Layered_mixedLobes_WeightsCalculator : IMxLayeredWeightCalculator
{
    void calculateWeights<let TBsdfCount : int, let TLayerCount : int, TLayeredMaterialData : IMxLayeredMaterialData>(
        const TLayeredMaterialData data, out MxWeights<TBsdfCount> weights)
    {
        for (uint i = 0; i < TBsdfCount; i++) weights.weights[i] = float3(0.f);
    }
}

struct MxLayeredMaterial : IMaterial
{
    float3 baseColor;

    typealias UsedMaterialInstance = MxLayeredMaterialInstance<3, 1, TMxLayeredMaterialData, Layered_mixedLobes_WeightsCalculator>;
    typedef UsedMaterialInstance MaterialInstance;    

    float3 getAlbedo(float3 albedo)
    {
        return albedo;
    }

    [ForwardDerivativeOf(getAlbedo)]
    [TreatAsDifferentiable]
    DifferentialPair<float3> __fwd_d_getAlbedo(DifferentialPair<float3> dpAlbedo)
    {
        return diffPair(dpAlbedo.p, float3(1.f));
    }

    [BackwardDerivativeOf(getAlbedo)]
    [TreatAsDifferentiable]
    void __bwd_d_getAlbedo(inout DifferentialPair<float3> dpAlbedo, float3 dOut)
    {
        [unroll]
        for (int j = 0; j < 3; j++) outputBuffer[j + 6] += dOut[j];
    }
    
    [Differentiable]
    UsedMaterialInstance setupMaterialInstance(out MaterialInstanceData miData)
    {
        float3 albedo = getAlbedo(baseColor);
        UsedMaterialInstance mi;
        for (uint i = 0; i < 3; i++) miData.data[i] = albedo[i];
        return mi;
    }
}
