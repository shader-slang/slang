// `invariant` only applies to the vertex position output, so not to inputs.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage vertex

struct VSOut
{
    float4 pos : SV_Position;
    invariant float2 uv : TEXCOORD0;
/*CHECK:
    ^^^^^^^^^ 'invariant' is only allowed on a declaration with the 'SV_Position' semantic
    ^^^^^^^^^ only the vertex position output can be marked invariant
*/
};

VSOut main(float3 p : POSITION)
{
    invariant float scale = 2.0;
/*CHECK:
    ^^^^^^^^^ 'invariant' is only allowed on a declaration with the 'SV_Position' semantic
    ^^^^^^^^^ only the vertex position output can be marked invariant
*/
    VSOut output;
    output.pos = float4(p * scale, 1.0);
    output.uv = p.xy;
    return output;
}

float4 fragmentMain(invariant float4 pos : SV_Position) : SV_Target
/*CHECK:
                    ^^^^^^^^^ 'invariant' is only allowed on an 'out' or 'inout' parameter
                    ^^^^^^^^^ 'pos' is an input, so it cannot be marked invariant
*/
{
    return pos;
}
