// An entry point exchanges values with the pipeline rather than with a caller's memory, so none
// of its parameters may use a reference passing mode: `__ref`, `const __ref`, `__ref_readonly`,
// or `__ref_writeonly`. Each such parameter is reported. A borrow such as `__constref` has
// value semantics and remains valid.
//
// A rejected parameter still reaches the remaining entry-point checks, as an input for
// `__ref_readonly`, an output for `__ref_writeonly`, and both for `__ref`, so those checks report
// their own problems with it.

//DIAGNOSTIC_TEST:SIMPLE(diag=REF):-target spirv -entry refParam -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=REF):-target hlsl -entry refParam -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=CONSTREF):-target spirv -entry constRefParam -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=RO):-target spirv -entry readOnlyParam -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=WO):-target spirv -entry writeOnlyParam -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=UNIFORM):-target spirv -entry uniformRefParam -stage compute -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=MULTI):-target spirv -entry multiRefParam -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=BORROW):-target spirv -entry constRefBorrowParam -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=RODEPTH):-target spirv -entry readOnlyDepth -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=WODEPTH):-target spirv -entry writeOnlyTwoDepths -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=REFPOS):-target spirv -entry refPosition -stage vertex -experimental-feature

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
//CONSTREF:                             ^^^^^ entry-point parameter 'color' cannot use the reference passing mode 'ref_readonly', because an entry point exchanges values with the pipeline rather than with a caller's memory location
{
    return color;
}

[shader("fragment")]
float4 readOnlyParam(__ref_readonly float4 color : COLOR0) : SV_Target
//RO:                                      ^^^^^ E38054
//RO:                                      ^^^^^ entry-point parameter 'color' cannot use the reference passing mode 'ref_readonly', because an entry point exchanges values with the pipeline rather than with a caller's memory location
{
    return color;
}

[shader("fragment")]
void writeOnlyParam(__ref_writeonly float4 color : SV_Target)
//WO:                                      ^^^^^ E38054
//WO:                                      ^^^^^ entry-point parameter 'color' cannot use the reference passing mode 'ref_writeonly', because an entry point exchanges values with the pipeline rather than with a caller's memory location
{
    color = float4(1.0);
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

[shader("fragment")]
float4 readOnlyDepth(__ref_readonly float depth : SV_Depth) : SV_Target
//RODEPTH:                                ^^^^^ E38054
//RODEPTH:                                ^^^^^ entry-point parameter 'depth' cannot use the reference passing mode 'ref_readonly'
//RODEPTH:                                ^^^^^ system value semantic 'SV_Depth' cannot be used as input in 'pixel' shader stage
{
    return depth;
}

[shader("fragment")]
float4 writeOnlyTwoDepths(
    __ref_writeonly float a : SV_Depth,
//WODEPTH:                ^ E38054
//WODEPTH:                ^ entry-point parameter 'a' cannot use the reference passing mode 'ref_writeonly'
    __ref_writeonly float b : SV_DepthGreaterEqual) : SV_Target
//WODEPTH:                ^ E38054
//WODEPTH:                ^ entry-point parameter 'b' cannot use the reference passing mode 'ref_writeonly'
//WODEPTH:                    ^^^^^^^^^^^^^^^^^^^^ a fragment entry point can declare at most one depth output, but 'SV_DepthGreaterEqual' conflicts with the earlier 'SV_Depth'
{
    a = 0;
    b = 0;
    return 0;
}

[shader("vertex")]
float4 refPosition(__ref float4 position : SV_Position) : SV_Position
//REFPOS:                       ^^^^^^^^ E38054
//REFPOS:                       ^^^^^^^^ entry-point parameter 'position' cannot use the reference passing mode 'ref'
//REFPOS:                       ^^^^^^^^ system value semantic 'SV_Position' cannot be used as input in 'vertex' shader stage
{
    return position;
}
