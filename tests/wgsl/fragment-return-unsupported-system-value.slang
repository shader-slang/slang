// A fragment shader's non-struct return value with a system value WGSL does not have
// (SV_StencilRef) is reported, as for a struct field, instead of becoming color output 0.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target wgsl -stage fragment -entry main

uint main(float4 p: SV_Position) : SV_StencilRef
//CHECK: E55202
{
    return 3;
}
