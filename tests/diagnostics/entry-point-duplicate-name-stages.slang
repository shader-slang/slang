// SPIR-V keeps one entry-point namespace per execution model, so a vertex and a fragment shader
// can both be named `Main` there (shader-slang/slang#14053). Metal and WGSL put every entry
// point into one namespace, so the same pair is an error on those targets.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl -whole-program

// SPIRV-DAG: OpEntryPoint Vertex %{{.*}} "Main"
// SPIRV-DAG: OpEntryPoint Fragment %{{.*}} "Main"

[shader("vertex")]
float4 Main(float4 position: POSITION) : SV_Position
/*CHECK:
       ^^^^ see the other entry point named 'Main'
*/
{
    return position;
}

[shader("fragment")]
float4 Main(float3 color: COLOR) : SV_Target
/*CHECK:
       ^^^^ more than one entry point is named 'Main' in the output for target
       ^^^^ entry point named 'Main' conflicts with another entry point
*/
{
    return float4(color, 1);
}
