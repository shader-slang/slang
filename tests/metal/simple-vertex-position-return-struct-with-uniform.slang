//TEST:SIMPLE(filecheck=METAL): -target metal -stage vertex -entry vertexMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage vertex -entry vertexMain

struct VertexIn
{
    float3 color;
}

//METAL: [position]]
//METALLIB: @vertexMain
struct VertexOut
{
    float4 position : SV_Position;
    float3 color;
}
// Vertex Shader which writes to position in a returned struct (as metal expects)

// METAL: EntryPointParams{{.*}} constant* entryPointParams{{.*}} {{\[\[}}buffer(0){{\]\]}}
VertexOut vertexMain(uniform VertexIn* args)
{
    VertexOut o;
    o.position = float4(1, 2, 3, 4);
    o.color = args->color;
    return o;
}
