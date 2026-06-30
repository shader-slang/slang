//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv

// Regression test for #8753.
//
// A vertex shader that returns a struct with no SV_Position member
// silently fails to drive the rasterizer; warn the user.

struct VSOutput
{
    float4 Pos; // missing SV_Position semantic -- bug
};

[shader("vertex")]
VSOutput vertexMain(uint vid : SV_VertexID)
/*diag:
         ^^^^^^^^^^ vertex shader 'vertexMain' has no output with the 'SV_Position' system value semantic
         ^^^^^^^^^^ vertex shader 'vertexMain' has no output with the 'SV_Position' system value semantic; the rasterizer will not receive valid vertex positions (add 'SV_Position' to a vertex output, or suppress with -warnings-disable 38052)
*/
{
    VSOutput output;
    output.Pos = float4(0, 0, 0, 1);
    return output;
}

// Void return + out parameter without SV_Position should also warn.
[shader("vertex")]
void outParamMissing(uint vid : SV_VertexID, out float4 color : COLOR)
/*diag:
     ^^^^^^^^^^^^^^^ vertex shader 'outParamMissing' has no output with the 'SV_Position' system value semantic
     ^^^^^^^^^^^^^^^ vertex shader 'outParamMissing' has no output with the 'SV_Position' system value semantic; the rasterizer will not receive valid vertex positions (add 'SV_Position' to a vertex output, or suppress with -warnings-disable 38052)
*/
{
    color = float4(1);
}
