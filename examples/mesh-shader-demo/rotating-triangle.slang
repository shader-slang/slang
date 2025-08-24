// rotating-triangle.slang
// Mesh shader demo showing a rotating triangle

struct UniformBufferObject {
    float time;
    float3 colors[3]; // Array of 3 colors for the triangle vertices
};

ConstantBuffer<UniformBufferObject> ubo;

struct MeshOutput {
    float4 Position : SV_Position;
    float3 Color : COLOR;
};

// Mesh shader that generates a rotating triangle
[shader("mesh")]
[numthreads(1, 1, 1)]
[outputtopology("triangle")]
void main(
    uint threadID : SV_GroupIndex,
    out indices uint3 Triangles[1],
    out vertices MeshOutput Vertices[3]
) {
    SetMeshOutputCounts(3, 1);
    
    // Define three vertices of a triangle
    float2 positions[3] = {
        float2( 0.0,  0.5), // Top
        float2( 0.5, -0.5), // Right
        float2(-0.5, -0.5)  // Left
    };

    // Get colors from constant buffer
    float3 colors[3] = {
        ubo.colors[0], // Dynamic color from CPU
        ubo.colors[1], // Dynamic color from CPU
        ubo.colors[2]  // Dynamic color from CPU
    };
    
    // Assign vertex data with rotation based on time
    for (uint i = 0; i < 3; i++)
    {
        float t = ubo.time;
        float2 p = positions[i];
    
        // Apply rotation matrix
        float x = p.x * cos(t) - p.y * sin(t);
        float y = p.x * sin(t) + p.y * cos(t);
    
        Vertices[i].Position = float4(x, y, 0.0, 1.0);
        Vertices[i].Color = colors[i];
    }
    
    // Define triangle indices
    Triangles[0] = uint3(0, 1, 2);
}

// Fragment shader that outputs the interpolated vertex colors
[shader("fragment")]
float4 fragmentMain(MeshOutput input) : SV_Target
{
    return float4(input.Color, 1.0);
} 