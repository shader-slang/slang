//TEST:SIMPLE(filecheck=SPIRV): -entry main -stage vertex -target spirv -emit-spirv-directly
//TEST:SIMPLE(filecheck=GLSL): -entry main -stage vertex -target glsl
//TEST:SIMPLE(filecheck=METAL): -entry main -stage vertex -target metal
//TEST:SIMPLE(filecheck=WGSL): -entry main -stage vertex -target wgsl

struct VSInput
{
    uint vertexID : SV_VulkanVertexID;
    uint instanceID : SV_VulkanInstanceID;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};

[shader("vertex")]
VSOutput main(VSInput input)
{
    VSOutput output;
    output.position = float4(input.vertexID + input.instanceID);

    // SPIRV-DAG: BuiltIn InstanceIndex
    // SPIRV-DAG: BuiltIn VertexIndex
    // SPIRV-NOT: BuiltIn BaseInstance
    // SPIRV-NOT: BuiltIn BaseVertex

    // GLSL-NOT: GL_ARB_shader_draw_parameters
    // GLSL-DAG: gl_VertexIndex
    // GLSL-NOT: gl_BaseVertex
    // GLSL-DAG: gl_InstanceIndex
    // GLSL-NOT: gl_BaseInstance

    // METAL-DAG: vertex_id
    // METAL-DAG: instance_id

    // WGSL-DAG: vertex_index
    // WGSL-DAG: instance_index

    return output;
}

