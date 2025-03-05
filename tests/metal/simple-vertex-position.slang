//TEST:SIMPLE(filecheck=METAL): -target metal -stage vertex -entry vertexMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage vertex -entry vertexMain
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage vertex -entry vertexMain
//TEST:SIMPLE(filecheck=WGSLSPIRV): -target wgsl-spirv-asm -stage vertex -entry vertexMain

//METAL: position
//METALLIB: @vertexMain

//WGSL: @builtin(position)
//WGSLSPIRV: %vertexMain = OpFunction

// Vertex Shader which writes to position
void vertexMain(out float4 position : SV_Position)
{
    position = float4(0.6, 0.1, 0.6, 0.33);
}
