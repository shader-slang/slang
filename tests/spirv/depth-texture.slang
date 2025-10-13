//TEST:SIMPLE(filecheck=SPIRV): -target spirv
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry psMain -stage pixel

// SPIRV: OpTypeImage %{{.*}} 2D 1 0 0 1 Unknown

[[vk::binding(0, 0)]] DepthTexture2D depthTexture : register(t0);
[[vk::binding(1, 0)]] SamplerComparisonState comparisonSampler : register(s0);

[[shader("pixel")]]
float4 psMain(
    float4 position : SV_POSITION,
    float2 texCoord : TEXCOORD0
) : SV_TARGET {
    float comparison = depthTexture.SampleCmp(
        comparisonSampler,
        texCoord,
        0.5
    );

    if (comparison != 0.0) {
        return float4(1.0, 1.0, 1.0, 1.0);
    } else {
        return float4(0.0, 0.0, 0.0, 1.0);
    }
}
