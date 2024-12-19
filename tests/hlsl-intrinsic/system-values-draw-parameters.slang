//TEST:SIMPLE(filecheck=CHECK_SPIRV): -entry main -stage compute -target spirv
//TEST:SIMPLE(filecheck=CHECK_GLSL): -entry main -stage compute -target glsl
//TEST:SIMPLE(filecheck=CHECK_HLSL): -entry main -stage compute -target hlsl
//TEST:SIMPLE(filecheck=CHECK_METAL): -entry main -stage compute -target metal

struct VSInput
{
    uint vertexID : SV_VertexID;
    uint instanceID : SV_InstanceID;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};

VSOutput main(VSInput input, 
              uint startVertexLocation : SV_StartVertexLocation, 
              uint startInstanceLocation : SV_StartInstanceLocation)
{
    VSOutput output;

    float x = (float)(input.vertexID + startVertexLocation) * 0.1f;
    float y = (float)(input.instanceID + startInstanceLocation) * 0.2f;
    output.position = float4(x, y, 0.0f, 1.0f);

    // CHECK_SPIRV: BuiltIn BaseInstance
    // CHECK_GLSL: gl_BaseInstance
    // CHECK_HLSL: SV_StartInstanceLocation
    // CHECK_METAL: base_instance

    return output;
}

