// An entry point exchanges values with the pipeline rather than with a caller's memory, so none
// of its parameters may use the `__ref` passing mode, with or without `const`. Each such
// parameter is reported. A borrow such as `__constref` has value semantics and remains valid.

//DIAGNOSTIC_TEST:SIMPLE(diag=REF):-target spirv -entry refParam -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=REF):-target hlsl -entry refParam -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=CONSTREF):-target spirv -entry constRefParam -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=UNIFORM):-target spirv -entry uniformRefParam -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=MULTI):-target spirv -entry multiRefParam -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(diag=BORROW):-target spirv -entry constRefBorrowParam -stage fragment

[shader("fragment")]
float4 refParam(__ref float4 color : COLOR0) : SV_Target
//REF:                       ^^^^^ E38054
//REF:                       ^^^^^ entry-point parameter 'color' cannot use the reference passing mode 'ref', because an entry point exchanges values with the pipeline rather than with a caller's memory location
{
    return color;
}

[shader("fragment")]
float4 constRefParam(const __ref float4 color : COLOR0) : SV_Target
//CONSTREF:                             ^^^^^ E38054
//CONSTREF:                             ^^^^^ entry-point parameter 'color' cannot use the reference passing mode 'ref', because an entry point exchanges values with the pipeline rather than with a caller's memory location
{
    return color;
}

RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void uniformRefParam(uniform __ref int value)
//UNIFORM:                             ^^^^^ E38054
//UNIFORM:                             ^^^^^ entry-point parameter 'value' cannot use the reference passing mode 'ref', because an entry point exchanges values with the pipeline rather than with a caller's memory location
{
    output[0] = value;
}

[shader("fragment")]
float4 multiRefParam(__ref float4 a : COLOR0, float4 ok : COLOR1, __ref float4 b : COLOR2) : SV_Target
//MULTI:                          ^ E38054
//MULTI:                          ^ entry-point parameter 'a' cannot use the reference passing mode 'ref'
//MULTI:                                                                       ^ E38054
//MULTI:                                                                       ^ entry-point parameter 'b' cannot use the reference passing mode 'ref'
{
    return a + ok + b;
}

[shader("fragment")]
float4 constRefBorrowParam(__constref float4 color : COLOR0) : SV_Target
{
    return color;
}
