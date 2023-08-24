//TEST_IGNORE_FILE:

struct MxWeights<let TBsdfCount : int>
{
    float3 weights[TBsdfCount];
}

interface IMxLayeredMaterialData
{
    int getBsdfCount();
    int getLayerCount();
}

interface IMxLayeredWeightCalculator
{
    void calculateWeights<let TBsdfCount : int, let TLayerCount: int, TLayeredMaterialData : IMxLayeredMaterialData>(
        const TLayeredMaterialData data, out MxWeights<TBsdfCount> weights);
}
