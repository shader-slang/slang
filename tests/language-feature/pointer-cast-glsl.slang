//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main

// Test that pointer casts in GLSL generate constructor-style casts instead of C-style casts
// This addresses issue https://github.com/shader-slang/slang/issues/7838

// GLSL: BufferPointer{{[a-zA-Z0-9_]+}}({{[a-zA-Z0-9_]+}}_0)

[shader("vertex")]
float4 main(uint vertexID : SV_VertexID, uint64_t address, float4* pointer) : SV_Position
{
    // This should generate BufferPointer(address) instead of (BufferPointer)address in GLSL
    let buffer1 = ConstBufferPointer<float4>(address);
    let buffer2 = ConstBufferPointer<float4>(pointer);
    return buffer1[vertexID] + buffer2[vertexID];
}
