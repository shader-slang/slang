//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile sm_6_6 -stage mesh -entry main

struct MeshOutput
{
    float4 position : SV_Position;
    float3 color : COLOR0;
}

// Uniform variable for time-based animation
uniform uint iTime;

[shader("mesh")]
[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(
    in uint tig : SV_GroupIndex,
    OutputVertices<MeshOutput, 3> vertices,
    OutputIndices<uint3, 1> triangles
)
{
    // Set the number of vertices and primitives
    SetMeshOutputCounts(3, 1);

    // Define the triangle indices
    triangles[0] = uint3(0, 1, 2);

    // Use uiColor as in the original shader
    // This line should now work after the fix - it was causing the "subscript had no getter" error
    uint uiColor = triangles[0].x + iTime;

    // Create triangle vertices with time-based animation
    float timeOffset = float(iTime) * 0.01; // Convert to float and scale
    float colorOffset = float(uiColor) * 0.005; // Use uiColor for color variation

    // Vertex 0 - Red vertex (top) with uiColor influence
    vertices[0].position = float4(0.0, 0.5 + sin(timeOffset) * 0.1, 0.0, 1.0);
    vertices[0].color = float3(1.0, (sin(colorOffset) + 1.0) * 0.5, 0.0); // Red with green modulation from uiColor

    // Vertex 1 - Green vertex (bottom left)
    vertices[1].position = float4(-0.5, -0.5, 0.0, 1.0);
    vertices[1].color = float3(0.0, 1.0, 0.0); // Green

    // Vertex 2 - Blue vertex (bottom right)
    vertices[2].position = float4(0.5, -0.5, 0.0, 1.0);
    vertices[2].color = float3(0.0, 0.0, 1.0); // Blue
}

// CHECK: OpCapability MeshShadingEXT
// CHECK: OpExtension "SPV_EXT_mesh_shader"
// CHECK: OpEntryPoint MeshEXT %main "main"
// CHECK: OpExecutionMode %main OutputPrimitivesEXT 1
// CHECK: OpExecutionMode %main OutputVertices 3
// CHECK: OpExecutionMode %main LocalSize 1 1 1
// CHECK: OpExecutionMode %main OutputTrianglesEXT
