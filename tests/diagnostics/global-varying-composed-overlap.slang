// Every entry point owns all module-scope varyings of its module. Two vertex shaders in this module
// both write `gPos`, so each compiles on its own. A program that contains both entry points would
// have two owners for `gPos`, so composing them is an error. Module checking also warns that the
// module has two `[shader]` functions, even though both are for the same stage.

//DIAGNOSTIC_TEST:SIMPLE(diag=COMPOSED):-target spirv-asm -warnings-disable 30627
//DIAGNOSTIC_TEST:SIMPLE(diag=SINGLE):-target spirv-asm -entry vsFirst -stage vertex -warnings-disable 30627

out float4 gPos : SV_Position;

[shader("vertex")]
void vsFirst()
/*COMPOSED:
     ^^^^^^^ 'vsFirst' is declared here
     ^^^^^^^ 'vsFirst' is declared here
*/
/*SINGLE:
     ^^^^^^^ 'vsFirst' is declared here
*/
{
    gPos = float4(0, 0, 0, 1);
}

[shader("vertex")]
void vsSecond()
/*COMPOSED:
     ^^^^^^^^ several '[shader]' functions in a module with module-scope varyings
     ^^^^^^^^ 'vsSecond' and 'vsFirst' both have a '[shader]' attribute in a module that declares module-scope varyings; if both become entry points of one program, each would own all of those varyings
     ^^^^^^^^ two entry points of a program come from a module with module-scope varyings
     ^^^^^^^^ entry points 'vsSecond' and 'vsFirst' both come from module 'global-varying-composed-overlap', which declares module-scope varyings; each entry point owns all of them, so a program can contain only one of these entry points
*/
/*SINGLE:
     ^^^^^^^^ several '[shader]' functions in a module with module-scope varyings
     ^^^^^^^^ 'vsSecond' and 'vsFirst' both have a '[shader]' attribute in a module that declares module-scope varyings; if both become entry points of one program, each would own all of those varyings
*/
{
    gPos = float4(1, 1, 1, 1);
}
