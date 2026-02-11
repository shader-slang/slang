//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=MTL): -target metal -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=DXIL): -target dxil -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=SPV): -target spirv


// Test that DescriptorHandle<T> types can be used with dynamic dispatch
// This addresses issue #9131

// SPV: %computeMain
// HLSL: ResourceDescriptorHeap[{{.*}}]
// HLSL: SamplerDescriptorHeap[{{.*}}]
// DXIL: computeMain
// CUDA: computeMain
// MTL: computeMain

public interface ILight {
    float2 sample(float2 direction);
};

public struct LightDescriptor {
    LightType lightType;
    uint dataOffset;
};

public enum LightType : uint {
    Point = 0,
    Area = 1,
    Spot = 2,
    Dome = 3,
};

public struct DomeLight : ILight {
    DescriptorHandle<Texture2D<float4>> hdrTexture;
    DescriptorHandle<Texture2D<float>> pdfTexture;
    DescriptorHandle<Texture2D<float2>> sampledDirectionsTexture;
    DescriptorHandle<SamplerState> linearSampler;
    DescriptorHandle<SamplerState> pointSampler;
    DescriptorHandle<StructuredBuffer<float2>> bufferData;

    float2 sample(float2 direction) {
        return hdrTexture.Sample(linearSampler, direction).rg + bufferData[0];
    }
}

public struct NullLight : ILight {
    float2 sample(float2 direction) {
        return float2(0.0, 0.0);
    }
}

/**
 * Reads from light buffer to create
 * a light object based on its descriptor.
 */
public struct LightFactory {
    public static ILight getLight(LightDescriptor desc, ByteAddressBuffer lightData) {
        switch (desc.lightType) {
        case LightType.Dome:
            return lightData.Load<DomeLight>(desc.dataOffset);
        default:
            return NullLight();
        }
    }
};

// Required so the compiler doesn't optimize everything away.
[vk::binding(0, 0)]
ByteAddressBuffer lightData;

[vk::binding(1, 0)]
StructuredBuffer<LightDescriptor> descriptors;

uniform RWStructuredBuffer<float2> outputBuffer;

[shader("compute")]
[numthreads(4, 4, 1)]
void computeMain(uint3 id: SV_DispatchThreadID)
{
    LightDescriptor desc = descriptors[id.x];

    // Force the compiler to resolve the factory logic
    ILight light = LightFactory.getLight(desc, lightData);
    float2 color = light.sample(float2(0.0, 0.0));

    outputBuffer[id.x] = color;
}
