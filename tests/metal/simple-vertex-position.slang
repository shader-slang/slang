//TEST:SIMPLE(filecheck=METAL): -target metal -stage vertex -entry vertexMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage vertex -entry vertexMain

//METAL: [position]]
//METALLIB: @vertexMain

// Vertex Shader which writes to position
void vertexMain(out float4 position : SV_Position, in uint vertexID : SV_VertexID)
{
    position = float4(0.6, 0.1, 0.6, 0.33);
}
