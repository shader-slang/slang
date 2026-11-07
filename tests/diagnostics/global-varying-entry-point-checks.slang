// The semantic checker applies the entry-point signature rules to the module-scope varyings that
// an entry point owns, which are all the module-scope varyings of its module. `main` reads none of
// them and still must return `void`, because `gDepth` is one of its outputs, and `gIntMatrix` must
// have a valid varying type. `-no-codegen` stops compilation before IR, so these diagnostics come
// from the checker.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage fragment -no-codegen -warnings-disable 30627

out float gDepth : SV_Depth;
in int2x2 gIntMatrix;
/*CHECK:
          ^^^^^^^^^^ type cannot be used as entry-point varying for this target
          ^^^^^^^^^^ type 'matrix<int,2,2>' cannot be used as input global variable 'gIntMatrix' of entry point 'main' when targeting spirv
*/

float4 main() : SV_Target
/*CHECK:
       ^^^^ entry point must return void with global outputs
       ^^^^ entry point must return 'void' when global output variables are present.
*/
{
    return float4(1);
}
