// An entry point exchanges values with the pipeline and its resource bindings rather than with a
// caller's memory, so its parameters may only use the passing modes those sources support: a
// varying parameter may be `in`, `out`, or `inout`, and a uniform parameter may only be `in`.
// Every other mode is rejected, including the reference modes (`__ref`, `const __ref`,
// `__ref_readonly`, `__ref_writeonly`) and the explicit borrows (`__borrow` and its legacy alias
// `__constref`). Each disallowed parameter is reported.
//
// A rejected parameter still reaches the remaining entry-point checks, as an input for
// `__ref_readonly`, an output for `__ref_writeonly`, and both for `__ref` and `inout`, so those
// checks report their own problems with it.

//DIAGNOSTIC_TEST:SIMPLE(diag=REF):-target spirv -entry refParam -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=REF):-target hlsl -entry refParam -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=CONSTREF):-target spirv -entry constRefParam -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=RO):-target spirv -entry readOnlyParam -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=WO):-target spirv -entry writeOnlyParam -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=UNIFORM):-target spirv -entry uniformRefParam -stage compute -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=MULTI):-target spirv -entry multiRefParam -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=BORROW):-target spirv -entry constRefBorrowParam -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=BORROWV):-target spirv -entry borrowVaryingParam -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=BORROWRES):-target spirv -entry borrowResourceParam -stage compute -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=OUTUNIF):-target spirv -entry outUniformParam -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=INOUTUNIF):-target spirv -entry inoutUniformParam -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=RODEPTH):-target spirv -entry readOnlyDepth -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=WODEPTH):-target spirv -entry writeOnlyTwoDepths -stage fragment -experimental-feature
//DIAGNOSTIC_TEST:SIMPLE(diag=REFPOS):-target spirv -entry refPosition -stage vertex -experimental-feature

[shader("fragment")]
float4 refParam(__ref float4 color : COLOR0) : SV_Target
//REF:                       ^^^^^ E38054
//REF:                       ^^^^^ entry-point parameter 'color' cannot use the 'ref' passing mode; a varying entry-point parameter can only use 'in', 'out', or 'inout', because an entry point exchanges values with the pipeline rather than with a caller's memory location
{
    return color;
}

[shader("fragment")]
float4 constRefParam(const __ref float4 color : COLOR0) : SV_Target
//CONSTREF:                             ^^^^^ E38054
//CONSTREF:                             ^^^^^ entry-point parameter 'color' cannot use the 'ref_readonly' passing mode; a varying entry-point parameter can only use 'in', 'out', or 'inout', because an entry point exchanges values with the pipeline rather than with a caller's memory location
{
    return color;
}

[shader("fragment")]
float4 readOnlyParam(__ref_readonly float4 color : COLOR0) : SV_Target
//RO:                                      ^^^^^ E38054
//RO:                                      ^^^^^ entry-point parameter 'color' cannot use the 'ref_readonly' passing mode; a varying entry-point parameter can only use 'in', 'out', or 'inout', because an entry point exchanges values with the pipeline rather than with a caller's memory location
{
    return color;
}

[shader("fragment")]
void writeOnlyParam(__ref_writeonly float4 color : SV_Target)
//WO:                                      ^^^^^ E38054
//WO:                                      ^^^^^ entry-point parameter 'color' cannot use the 'ref_writeonly' passing mode; a varying entry-point parameter can only use 'in', 'out', or 'inout', because an entry point exchanges values with the pipeline rather than with a caller's memory location
{
    color = float4(1.0);
}

RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void uniformRefParam(uniform __ref int value)
//UNIFORM:                             ^^^^^ E38054
//UNIFORM:                             ^^^^^ entry-point parameter 'value' cannot use the 'ref' passing mode; a uniform entry-point parameter can only use 'in', because its value comes from a constant buffer or a resource binding rather than from a caller
{
    output[0] = value;
}

[shader("fragment")]
float4 multiRefParam(__ref float4 a : COLOR0, float4 ok : COLOR1, __ref float4 b : COLOR2) : SV_Target
//MULTI:                          ^ E38054
//MULTI:                          ^ entry-point parameter 'a' cannot use the 'ref' passing mode
//MULTI:                                                                       ^ E38054
//MULTI:                                                                       ^ entry-point parameter 'b' cannot use the 'ref' passing mode
{
    return a + ok + b;
}

[shader("fragment")]
float4 constRefBorrowParam(__constref float4 color : COLOR0) : SV_Target
//BORROW:                                    ^^^^^ E38054
//BORROW:                                    ^^^^^ entry-point parameter 'color' cannot use the 'borrow' passing mode; a varying entry-point parameter can only use 'in', 'out', or 'inout', because an entry point exchanges values with the pipeline rather than with a caller's memory location
{
    return color;
}

[shader("fragment")]
float4 borrowVaryingParam(__borrow float4 color : COLOR0) : SV_Target
//BORROWV:                                ^^^^^ E38054
//BORROWV:                                ^^^^^ entry-point parameter 'color' cannot use the 'borrow' passing mode; a varying entry-point parameter can only use 'in', 'out', or 'inout', because an entry point exchanges values with the pipeline rather than with a caller's memory location
{
    return color;
}

RWStructuredBuffer<float4> borrowResourceBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void borrowResourceParam(__borrow RWStructuredBuffer<float4> buffer)
//BORROWRES:                                                 ^^^^^^ E38054
//BORROWRES:                                                 ^^^^^^ entry-point parameter 'buffer' cannot use the 'borrow' passing mode; a uniform entry-point parameter can only use 'in', because its value comes from a constant buffer or a resource binding rather than from a caller
{
    buffer[0] = float4(1.0);
}

[shader("fragment")]
void outUniformParam(out uniform float4 value : SV_Target)
//OUTUNIF:                              ^^^^^ E38054
//OUTUNIF:                              ^^^^^ entry-point parameter 'value' cannot use the 'out' passing mode; a uniform entry-point parameter can only use 'in', because its value comes from a constant buffer or a resource binding rather than from a caller
{
    value = float4(1.0);
}

struct InoutUniformStruct
{
    float4 v;
}

[shader("fragment")]
float4 inoutUniformParam(inout uniform InoutUniformStruct u) : SV_Target
//INOUTUNIF:                                              ^ E38054
//INOUTUNIF:                                              ^ entry-point parameter 'u' cannot use the 'inout' passing mode; a uniform entry-point parameter can only use 'in', because its value comes from a constant buffer or a resource binding rather than from a caller
{
    u.v += 1;
    return u.v;
}

[shader("fragment")]
float4 readOnlyDepth(__ref_readonly float depth : SV_Depth) : SV_Target
//RODEPTH:                                ^^^^^ E38054
//RODEPTH:                                ^^^^^ entry-point parameter 'depth' cannot use the 'ref_readonly' passing mode; a varying entry-point parameter can only use 'in', 'out', or 'inout'
//RODEPTH:                                ^^^^^ system value semantic 'SV_Depth' cannot be used as input in 'pixel' shader stage
{
    return depth;
}

[shader("fragment")]
float4 writeOnlyTwoDepths(
    __ref_writeonly float a : SV_Depth,
//WODEPTH:                ^ E38054
//WODEPTH:                ^ entry-point parameter 'a' cannot use the 'ref_writeonly' passing mode; a varying entry-point parameter can only use 'in', 'out', or 'inout', because an entry point exchanges values with the pipeline rather than with a caller's memory location
    __ref_writeonly float b : SV_DepthGreaterEqual) : SV_Target
//WODEPTH:                ^ E38054
//WODEPTH:                ^ entry-point parameter 'b' cannot use the 'ref_writeonly' passing mode; a varying entry-point parameter can only use 'in', 'out', or 'inout', because an entry point exchanges values with the pipeline rather than with a caller's memory location
//WODEPTH:                    ^^^^^^^^^^^^^^^^^^^^ a fragment entry point can declare at most one depth output, but 'SV_DepthGreaterEqual' conflicts with the earlier 'SV_Depth'
{
    a = 0;
    b = 0;
    return 0;
}

[shader("vertex")]
float4 refPosition(__ref float4 position : SV_Position) : SV_Position
//REFPOS:                       ^^^^^^^^ E38054
//REFPOS:                       ^^^^^^^^ entry-point parameter 'position' cannot use the 'ref' passing mode; a varying entry-point parameter can only use 'in', 'out', or 'inout'
//REFPOS:                       ^^^^^^^^ system value semantic 'SV_Position' cannot be used as input in 'vertex' shader stage
{
    return position;
}
