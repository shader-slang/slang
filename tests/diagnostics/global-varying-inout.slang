// A module-scope `inout` variable is both an input and an output of the entry points that own it,
// as an `inout` parameter is, so its system-value semantic is checked in both directions and its
// type is checked as an "input/output" varying.

//DIAGNOSTIC_TEST:SIMPLE(diag=SEMANTIC):-target spirv -entry main -stage vertex -no-codegen -warnings-disable 30627
//DIAGNOSTIC_TEST:SIMPLE(diag=TYPE):-target spirv -entry main -stage fragment -no-codegen -warnings-disable 30627 -DCHECK_TYPE

#ifndef CHECK_TYPE
inout float4 gPosition : SV_Position;
/*SEMANTIC:
             ^^^^^^^^^ system value semantic 'SV_Position' cannot be used as input in 'vertex' shader stage
             ^^^^^^^^^ 'gPosition' belongs to entry point 'main'
*/

void main()
{
    gPosition = gPosition * 2;
}
#else
inout int2x2 gMatrix;
/*TYPE:
             ^^^^^^^ type cannot be used as entry-point varying for this target
             ^^^^^^^ type 'matrix<int,2,2>' cannot be used as input/output global variable 'gMatrix' of entry point 'main'
*/

void main()
{
    gMatrix = gMatrix * 2;
}
#endif
