// A fragment entry point can write at most one depth output, whether it declares depth outputs as
// parameters or owns them as module-scope `out` variables.

//DIAGNOSTIC_TEST:SIMPLE(diag=GLOBALS):-target spirv -entry main -stage fragment -warnings-disable 30627
//DIAGNOSTIC_TEST:SIMPLE(diag=MIXED):-target spirv -entry main -stage fragment -warnings-disable 30627 -DMIXED

out float gDepth : SV_Depth;
//MIXED:  ^^^^^^ module-scope varying 'gDepth' is declared here
//MIXED:           ^^^^^^^^ a fragment entry point can declare at most one depth output, but 'SV_Depth' conflicts with the earlier 'SV_DepthLessEqual'

#ifndef MIXED
out float gDepthGreater : SV_DepthGreaterEqual;
/*GLOBALS:
                          ^^^^^^^^^^^^^^^^^^^^ a fragment entry point can declare at most one depth output, but 'SV_DepthGreaterEqual' conflicts with the earlier 'SV_Depth'
*/

void main()
{
    gDepth = 0.5;
    gDepthGreater = 0.5;
}
#else
void main(out float d : SV_DepthLessEqual)
//MIXED:            ^ entry point declares both varying parameters and module-scope varyings
//MIXED:            ^ 'd' is a varying parameter of an entry point that also owns module-scope varyings; this will be an error in a future compiler release
{
    gDepth = 0.5;
    d = 0.5;
}
#endif
