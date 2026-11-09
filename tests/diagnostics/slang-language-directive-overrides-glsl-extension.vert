// A valid `#language` directive selects Slang even when the file extension would select GLSL.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#language 2026
//CHECK: source-language directive selects slang and overrides the file-extension-implied glsl source language
//CHECK: the source directive selects slang instead of glsl

[shader("vertex")]
float4 main(float4 position : POSITION) : SV_Position
{
    return position;
}
