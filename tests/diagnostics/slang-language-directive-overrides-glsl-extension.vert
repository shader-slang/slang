// A valid `#language` directive selects Slang even when the file extension would select GLSL.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#language 2026
//CHECK: source-language directive overrides the language selected for this input
//CHECK: the source directive takes precedence over the requested or file-extension-implied language

[shader("vertex")]
float4 main(float4 position : POSITION) : SV_Position
{
    return position;
}
