// An entry point that owns module-scope varyings should not also declare varying parameters, so
// each such parameter gets a warning, whether or not it has a system-value semantic. Uniform
// parameters, including resources that are uniform implicitly, are allowed. An entry point whose
// module has no module-scope varyings is unaffected.

//DIAGNOSTIC_TEST:SIMPLE(diag=MIXED):-target spirv -entry main -stage vertex -no-codegen -warnings-disable 30627
//DIAGNOSTIC_TEST:SIMPLE(diag=PARAMS_ONLY):-target spirv -entry main -stage vertex -no-codegen -DNO_GLOBALS

#ifndef NO_GLOBALS
in float4 gColor;
//MIXED:  ^^^^^^ module-scope varying 'gColor' is declared here
//MIXED:  ^^^^^^ module-scope varying 'gColor' is declared here
#endif

float4 main(
    float4 color : COLOR,
//MIXED:   ^^^^^ entry point declares both varying parameters and module-scope varyings
//MIXED:   ^^^^^ 'color' is a varying parameter of an entry point that also owns module-scope varyings; this will be an error in a future compiler release
    uint vertexID : SV_VertexID,
//MIXED: ^^^^^^^^ entry point declares both varying parameters and module-scope varyings
//MIXED: ^^^^^^^^ 'vertexID' is a varying parameter of an entry point that also owns module-scope varyings; this will be an error in a future compiler release
    uniform float4 tint,
    Texture2D texture) : SV_Position
{
    return color * tint + float(vertexID);
}
