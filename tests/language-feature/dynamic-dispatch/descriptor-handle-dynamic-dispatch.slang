//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain

// Test that DescriptorHandle<T> types can be used with dynamic dispatch
// This addresses issue #9131
// repro compiler option: ./slangc ./tests/language-feature/dynamic-dispatch/descriptor-handle-dynamic-dispatch.slang -target glsl -stage compute -entry main -o dynamic.glsl -gminimal

// CHECK-NOT: error 41014
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
    float2 sample(float2 direction) {
        return hdrTexture.Sample(linearSampler, direction).rg;
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

// 5. ENTRY POINT
// Required so the compiler doesn't optimize everything away.
[vk::binding(0, 0)]
ByteAddressBuffer lightData;

[vk::binding(1, 0)]
StructuredBuffer<LightDescriptor> descriptors;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 id: SV_DispatchThreadID)
{
    LightDescriptor desc = descriptors[id.x];

    // Force the compiler to resolve the factory logic
    ILight light = LightFactory.getLight(desc, lightData);
    float2 color = light.sample(float2(0.0, 0.0));
}