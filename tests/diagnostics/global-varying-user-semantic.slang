// A user semantic on a module-scope varying itself is lowered as a system value for SPIR-V, GLSL
// and WGSL, so those targets reject it in the checker. HLSL and Metal emit the user semantic as
// written and accept it. A struct-typed module-scope varying can carry user semantics on its
// fields on every target, as a parameter can.

//DIAGNOSTIC_TEST:SIMPLE(diag=USER):-target spirv -entry vsMain -stage vertex -no-codegen -warnings-disable 30627
//DIAGNOSTIC_TEST:SIMPLE(diag=USER_GLSL):-target glsl -entry vsMain -stage vertex -no-codegen -warnings-disable 30627
//DIAGNOSTIC_TEST:SIMPLE(diag=USER_WGSL):-target wgsl -entry vsMain -stage vertex -no-codegen -warnings-disable 30627
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry vsMain -stage vertex -warnings-disable 30627
//TEST:SIMPLE(filecheck=METAL):-target metal -entry vsMain -stage vertex -warnings-disable 30627
//TEST:SIMPLE(filecheck=FIELDS):-target spirv-asm -entry vsFields -stage vertex -warnings-disable 30627 -DUSE_FIELDS

#ifndef USE_FIELDS
in float4 gTexCoord : TEXCOORD0;
/*USER:
                      ^^^^^^^^^ user semantic on a module-scope varying is not supported when targeting spirv
                      ^^^^^^^^^ when targeting spirv, a module-scope varying can only have a system-value ('SV_') semantic; remove the semantic, or declare the varying with a struct type whose fields carry the semantics
          ^^^^^^^^^ 'gTexCoord' belongs to entry point 'vsMain'
*/
/*USER_GLSL:
                      ^^^^^^^^^ user semantic on a module-scope varying is not supported when targeting glsl
                      ^^^^^^^^^ when targeting glsl, a module-scope varying can only have a system-value ('SV_') semantic
          ^^^^^^^^^ 'gTexCoord' belongs to entry point 'vsMain'
*/
/*USER_WGSL:
                      ^^^^^^^^^ user semantic on a module-scope varying is not supported when targeting wgsl
                      ^^^^^^^^^ when targeting wgsl, a module-scope varying can only have a system-value ('SV_') semantic
          ^^^^^^^^^ 'gTexCoord' belongs to entry point 'vsMain'
*/
out float4 gColor : COLOR0;
/*USER:
                    ^^^^^^ user semantic on a module-scope varying is not supported when targeting spirv
                    ^^^^^^ when targeting spirv, a module-scope varying can only have a system-value ('SV_') semantic; remove the semantic, or declare the varying with a struct type whose fields carry the semantics
           ^^^^^^ 'gColor' belongs to entry point 'vsMain'
*/
/*USER_GLSL:
                    ^^^^^^ user semantic on a module-scope varying is not supported when targeting glsl
                    ^^^^^^ when targeting glsl, a module-scope varying can only have a system-value ('SV_') semantic
           ^^^^^^ 'gColor' belongs to entry point 'vsMain'
*/
/*USER_WGSL:
                    ^^^^^^ user semantic on a module-scope varying is not supported when targeting wgsl
                    ^^^^^^ when targeting wgsl, a module-scope varying can only have a system-value ('SV_') semantic
           ^^^^^^ 'gColor' belongs to entry point 'vsMain'
*/
in uint gVertexID : SV_VertexID;

//HLSL-DAG: gColor_0 : COLOR0
//HLSL-DAG: gTexCoord_0 : TEXCOORD0
//METAL-DAG: gColor_0 {{\[\[}}user(COLOR){{\]\]}}
//METAL-DAG: gTexCoord_0 {{\[\[}}attribute(0){{\]\]}}

[shader("vertex")]
void vsMain()
{
    gColor = gTexCoord + float(gVertexID);
}
#else
struct VertexInput
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD0;
};
in VertexInput gInput;

//FIELDS: OpEntryPoint Vertex
//FIELDS-DAG: OpDecorate %gInput_position Location 0
//FIELDS-DAG: OpDecorate %gInput_texCoord Location 1

[shader("vertex")]
float4 vsFields() : SV_Position
{
    return gInput.position + float4(gInput.texCoord, 0, 0);
}
#endif
