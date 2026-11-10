// The semantic on a module-scope varying itself is treated as a system value when it is lowered,
// so only `SV_` semantics are allowed there. A struct-typed module-scope varying can carry user
// semantics on its fields, as a parameter can.

//DIAGNOSTIC_TEST:SIMPLE(diag=USER):-target spirv -entry vsMain -stage vertex -no-codegen -warnings-disable 30627
//TEST:SIMPLE(filecheck=FIELDS):-target spirv-asm -entry vsFields -stage vertex -warnings-disable 30627 -DUSE_FIELDS

#ifndef USE_FIELDS
in float4 gTexCoord : TEXCOORD0;
/*USER:
                      ^^^^^^^^^ user semantic on a module-scope varying
                      ^^^^^^^^^ a module-scope varying can only have a system-value ('SV_') semantic; remove the semantic, or declare the varying with a struct type whose fields carry the semantics
*/
out float4 gColor : COLOR0;
/*USER:
                    ^^^^^^ user semantic on a module-scope varying
                    ^^^^^^ a module-scope varying can only have a system-value ('SV_') semantic; remove the semantic, or declare the varying with a struct type whose fields carry the semantics
*/
in uint gVertexID : SV_VertexID;

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

[shader("vertex")]
float4 vsFields() : SV_Position
{
    return gInput.position + float4(gInput.texCoord, 0, 0);
}
#endif
