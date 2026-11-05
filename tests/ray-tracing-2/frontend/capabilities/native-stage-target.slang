//TEST:SIMPLE: -entry main -target hlsl -no-codegen
//DIAGNOSTIC_TEST:SIMPLE(diag=METAL,non-exhaustive): -entry main -target metal -no-codegen

// A shared stage requirement does not enable native miss entry points on Metal.
[require(miss)]
void missOnlyHelper() {}

[shader("miss")]
void main()
/*METAL:
     ^^^^ unavailable features in entry point
*/
{
    missOnlyHelper();
}
