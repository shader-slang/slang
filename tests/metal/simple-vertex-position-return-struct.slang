//TEST:SIMPLE(filecheck=METAL): -target metal -stage vertex -entry vertexMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage vertex -entry vertexMain

//METAL: [position]]
//METALLIB: @vertexMain
struct VertexOut
{
    float4 position : SV_Position; 
}
// Vertex Shader which writes to position in a returned struct (as metal expects)
VertexOut vertexMain()
{
    VertexOut o;
    o.position = float4(1,2,3,4);
    return o;
}
