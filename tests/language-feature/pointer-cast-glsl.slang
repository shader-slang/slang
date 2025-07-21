//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main

// Test that pointer casts in GLSL generate constructor-style casts instead of C-style casts
// This addresses issue https://github.com/shader-slang/slang/issues/7838

//GLSL: BufferPointer__S1_2(address_0)

[shader("vertex")]
float4 main(uint vertexID : SV_VertexID, uint64_t address) : SV_Position
{
    // This should generate BufferPointer(address) instead of (BufferPointer)address in GLSL
    let buffer = ConstBufferPointer<float4>(address);
    return buffer[vertexID];
}