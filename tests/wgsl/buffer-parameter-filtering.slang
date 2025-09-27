//TEST:SIMPLE(filecheck=WGSL): -stage vertex -entry vertexMain -target wgsl

// Test that storage buffer parameters are filtered from function signatures 
// and converted to global variables

struct VertexData
{
    float4 position;
    float3 normal;
};

struct MaterialData
{
    float4 color;
    float roughness;
};

// These should become global variables with proper bindings
// WGSL: @binding({{[0-9]+}}) @group(0) var<storage, read_write> vertexBuffer_0 : array<VertexData_std430_0>;
RWStructuredBuffer<VertexData> vertexBuffer;

// WGSL: @binding({{[0-9]+}}) @group(0) var<storage, read_write> materialBuffer_0 : array<MaterialData_std430_0>;
RWStructuredBuffer<MaterialData> materialBuffer;

// Function that originally took buffer parameters should have them filtered out
// WGSL: fn processVertex_0( _S{{[0-9]+}} : u32) -> vec4<f32>
float4 processVertex(RWStructuredBuffer<VertexData> verts, RWStructuredBuffer<MaterialData> materials, uint index)
{
    VertexData vertex = verts[index];
    MaterialData material = materials[0];
    return vertex.position * material.color;
}

struct VertexOutput
{
    float4 position : SV_Position;
    float3 color : COLOR;
};

VertexOutput vertexMain(uint vertexId : SV_VertexID)
{
    VertexOutput output;
    
    // Function call should only pass non-buffer arguments
    // WGSL: processVertex_0(vertexId_0)
    output.position = processVertex(vertexBuffer, materialBuffer, vertexId);
    output.color = float3(1, 0, 0);
    
    return output;
}