// An entry point exchanges values with the pipeline rather than with a caller's memory, so none
// of its parameters may use a reference passing mode, however the mode is spelled.

//DIAGNOSTIC_TEST:SIMPLE(diag=REF):-target spirv -entry refParam -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=CONSTREF):-target spirv -entry constRefParam -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=UNIFORM):-target spirv -entry uniformRefParam -stage compute

[shader("fragment")]
float4 refParam(__ref float4 color : COLOR0) : SV_Target
//REF:                       ^^^^^ E38054
//REF:                       ^^^^^ entry-point parameter 'color' cannot use the reference passing mode 'ref'; an entry point exchanges values with the pipeline, so declare the parameter 'in', 'out', or 'inout' instead
{
    return color;
}

[shader("fragment")]
float4 constRefParam(const __ref float4 color : COLOR0) : SV_Target
//CONSTREF:                             ^^^^^ E38054
//CONSTREF:                             ^^^^^ entry-point parameter 'color' cannot use the reference passing mode 'ref'; an entry point exchanges values with the pipeline, so declare the parameter 'in', 'out', or 'inout' instead
{
    return color;
}

RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void uniformRefParam(uniform __ref int value)
//UNIFORM:                             ^^^^^ E38054
//UNIFORM:                             ^^^^^ entry-point parameter 'value' cannot use the reference passing mode 'ref'; an entry point exchanges values with the pipeline, so declare the parameter 'in', 'out', or 'inout' instead
{
    output[0] = value;
}
