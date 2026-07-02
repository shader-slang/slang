// Regression test for #8753 and #11884.
//
// A vertex shader that returns a struct with no SV_Position member silently fails to drive the
// rasterizer. Warning E38052 flags that, but a vertex shader may legitimately omit SV_Position
// when its output feeds a geometry/tessellation/mesh stage, so the warning is a `pedantic` group
// warning that is OFF by default and only fires under -Wpedantic.

// Off by default (no -Wpedantic): the warning must NOT appear.
//DIAGNOSTIC_TEST:SIMPLE(diag=OFF): -target spirv

// With -Wpedantic: the warning appears.
//DIAGNOSTIC_TEST:SIMPLE(diag=ON): -Wpedantic -target spirv

// With -Wpedantic but an explicit per-id disable: suppressed again.
//DIAGNOSTIC_TEST:SIMPLE(diag=OFF): -Wpedantic -Wno-vertex-shader-missing-sv-position -target spirv

// An explicit per-id enable turns this one warning on without -Wpedantic.
//DIAGNOSTIC_TEST:SIMPLE(diag=ON): -Wvertex-shader-missing-sv-position -target spirv

struct VSOutput
{
    float4 Pos; // missing SV_Position semantic -- bug
};

[shader("vertex")]
VSOutput vertexMain(uint vid : SV_VertexID)
/*ON:
         ^^^^^^^^^^ vertex shader 'vertexMain' has no output with the 'SV_Position' system value semantic
         ^^^^^^^^^^ vertex shader 'vertexMain' has no output with the 'SV_Position' system value semantic; if it feeds the rasterizer directly the rasterizer will not receive valid vertex positions (add 'SV_Position' to a vertex output)
*/
{
    VSOutput output;
    output.Pos = float4(0, 0, 0, 1);
    return output;
}

// Void return + out parameter without SV_Position should also warn.
[shader("vertex")]
void outParamMissing(uint vid : SV_VertexID, out float4 color : COLOR)
/*ON:
     ^^^^^^^^^^^^^^^ vertex shader 'outParamMissing' has no output with the 'SV_Position' system value semantic
     ^^^^^^^^^^^^^^^ vertex shader 'outParamMissing' has no output with the 'SV_Position' system value semantic; if it feeds the rasterizer directly the rasterizer will not receive valid vertex positions (add 'SV_Position' to a vertex output)
*/
{
    color = float4(1);
}
