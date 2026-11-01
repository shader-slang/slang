// A fragment shader's non-struct return value with a system value WGSL does not have
// (SV_StencilRef) is reported, as for a struct field, instead of becoming color output 0.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target wgsl -stage fragment -entry main
// The generated output field takes the entry point's location, so the error points at line 8.
//TEST:SIMPLE(filecheck=LOC): -target wgsl -stage fragment -entry main
// LOC: fragment-return-unsupported-system-value.slang:8:

uint main(float4 p: SV_Position) : SV_StencilRef
//CHECK: E55202
{
    return 3;
}
