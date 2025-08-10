//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry main

// Test that we can find the correct unused descriptor set index when there is `ParameterBlock` in the shader.

//CHECK: layout(binding = 2, set = 2)
//CHECK-NEXT: uniform texture2D  _slang_resource_heap

struct VSInput{
    float3 position;
}

struct VSOutput{
    float4 position : SV_Position;
}

struct CameraData{
    float3 position;
}

ParameterBlock<CameraData> cameraData;


struct Material{
    DescriptorHandle<Texture2D> texture;
    DescriptorHandle<SamplerState> samplerState;
}

StructuredBuffer<Material> materials;

struct FSOutput{
    float4 outColor : SV_Target0;
}


[shader("fragment")]
FSOutput main(){
    FSOutput output;
    output.outColor = materials[0].texture.Sample(materials[0].samplerState, float2(0));
    output.outColor.x *= cameraData.position.x;
    return output;
}