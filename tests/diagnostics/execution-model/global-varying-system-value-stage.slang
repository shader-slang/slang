// A module-scope varying with a system-value semantic is checked against the stage of each entry
// point that owns it, as an entry-point parameter with that semantic would be: an `in` variable as
// an input and an `out` variable as an output. An entry point owns every module-scope varying of
// its module, so the vertex run reports both variables even though `main` uses neither there. A
// stage that allows the semantics compiles normally.

//DIAGNOSTIC_TEST:SIMPLE(diag=VERTEX):-target spirv -entry main -stage vertex -no-codegen -warnings-disable 30627
//TEST:SIMPLE(filecheck=FRAGMENT):-target spirv -entry main -stage fragment -warnings-disable 30627 -DUSE_VARYINGS

in uint gPrim : SV_PrimitiveID;
/*VERTEX:
        ^^^^^ system value semantic 'SV_PrimitiveID' cannot be used as input in 'vertex' shader stage
        ^^^^^ 'gPrim' belongs to entry point 'main'
*/
out float gDepth : SV_Depth;
/*VERTEX:
          ^^^^^^ system value semantic 'SV_Depth' cannot be used as output in 'vertex' shader stage
          ^^^^^^ 'gDepth' belongs to entry point 'main'
*/

//FRAGMENT-DAG: OpDecorate %{{.*}} BuiltIn PrimitiveId
//FRAGMENT-DAG: OpDecorate %{{.*}} BuiltIn FragDepth

void main()
{
#ifdef USE_VARYINGS
    gDepth = float(gPrim);
#endif
}
