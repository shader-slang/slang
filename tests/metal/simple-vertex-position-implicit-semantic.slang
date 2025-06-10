//TEST:SIMPLE(filecheck=METAL): -target metal -stage vertex -entry vertexMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage vertex -entry vertexMain

//METAL: [position]]
//METALLIB: @vertexMain

// Vertex Shader which writes to position in a returned value 
float4 vertexMain() // : SV_Position
{
    return float4(1,2,3,4);
}
