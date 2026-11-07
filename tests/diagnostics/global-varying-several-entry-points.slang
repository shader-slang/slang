// Consider the example from #14375: one module holds a vertex shader that writes its outputs
// through module-scope varyings and a fragment shader that reads its input as a parameter and
// returns its result. Every entry point owns all module-scope varyings of its module, so
// `fsMain` owns `gPos` and `vA` as outputs even though it uses neither. The fragment entry point
// is therefore rejected: it returns a value while it has module-scope outputs, and `SV_Position`
// is not a fragment output. Each error carries a note that names the owning entry point. The
// vertex entry point compiles on its own.

//DIAGNOSTIC_TEST:SIMPLE(diag=FS):-target spirv -entry fsMain -stage fragment -no-codegen -warnings-disable 30627,38044
//TEST:SIMPLE(filecheck=VS):-target spirv -entry vsMain -stage vertex -warnings-disable 30627,38044

out float4 gPos : SV_Position;
/*FS:
           ^^^^ system value semantic 'SV_Position' cannot be used as output in 'pixel' shader stage
           ^^^^ 'gPos' belongs to entry point 'fsMain'
           ^^^^ 'gPos' belongs to entry point 'fsMain'
*/
out float3 vA;

//VS: OpEntryPoint Vertex
//VS: BuiltIn Position

[shader("vertex")]
void vsMain(float3 p : POSITION)
{
    gPos = float4(p, 1);
    vA = p;
}

[shader("fragment")]
float4 fsMain(float3 vA_in) : SV_Target
/*FS:
       ^^^^^^ entry point must return void with global outputs
       ^^^^^^ entry point must return 'void' when global output variables are present.
*/
{
    return float4(vA_in, 1);
}
