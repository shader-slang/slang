// SV_Position requires float4, using uint should error

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -skip-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl

[shader("vertex")]
uint vertexShaderBadPosition(uint id : SV_VertexID) : SV_Position
/*CHECK:
     ^^^^^^^^^^^^^^^^^^^^^^^ type 'uint' is not valid for system value semantic 'SV_Position'; expected 'vector<float,4>'
*/
{
    return id;
}
