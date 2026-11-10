// Every entry point owns all module-scope varyings of its module. `vsMain` and `fsMain` both read
// `gColor`, which has no system-value semantic and is a valid input of either stage, so each entry
// point passes its own checks and compiles on its own. A program that contains both has two owners
// for `gColor`, so composing them is an error even though they are for different stages. The entry
// points are selected with `-entry` rather than `[shader]`, so module checking does not warn.

//DIAGNOSTIC_TEST:SIMPLE(diag=COMPOSED):-target spirv-asm -entry vsMain -stage vertex -entry fsMain -stage fragment -warnings-disable 30627
//TEST:SIMPLE(filecheck=VS):-target spirv-asm -entry vsMain -stage vertex -warnings-disable 30627
//TEST:SIMPLE(filecheck=FS):-target spirv-asm -entry fsMain -stage fragment -warnings-disable 30627

in float4 gColor;
RWStructuredBuffer<float4> gResult;

void vsMain()
/*COMPOSED:
     ^^^^^^ 'vsMain' is declared here
*/
{
    gResult[0] = gColor;
}

void fsMain()
/*COMPOSED:
     ^^^^^^ two entry points of a program come from a module with module-scope varyings
     ^^^^^^ entry points 'fsMain' and 'vsMain' both come from module 'global-varying-composed-different-stages', which declares module-scope varyings; each entry point owns all of them, so a program can contain only one of these entry points
*/
{
    gResult[1] = gColor;
}

//VS: OpEntryPoint Vertex
//FS: OpEntryPoint Fragment
