//TEST:SIMPLE(filecheck=METAL): -target metal -stage vertex -entry vertexMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage vertex -entry vertexMain

//METAL: [[position]]
//METAL: [[user(AA)]]
//METALLIB: @vertexMain

// Vertex Shader which writes to position in a returned value (as metal expects), and as an out parameter which it doesn't
float4 vertexMain(out uint aa : AA) : SV_Position
{
    aa = 0;
    return float4(1,2,3,4);
}
