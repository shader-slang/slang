// SV_Position requires float4, using uint should error

// TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target metal -skip-codegen
//TEST:SIMPLE(filecheck=CHECK): -target hlsl

// CHECK: error {{.*}}SV_Position
[shader("vertex")]
uint vertexShaderBadPosition(uint id : SV_VertexID) : SV_Position
{
    return id;
}
