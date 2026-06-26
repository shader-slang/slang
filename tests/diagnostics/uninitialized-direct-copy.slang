//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -target spirv -entry computeMain

RWStructuredBuffer<float> output;

// Case 1 (the core bug): a direct copy of an uninitialized scalar local that
// is then written to an output buffer element. Previously the copy was
// silently elided so no warning was produced; now the uninitialized read is
// diagnosed at the point it is stored.
void direct_copy_to_local()
{
    float uninit;
    float copy = uninit;
    output[0] = copy;
//CHK:        ^ use of uninitialized variable
//CHK:        ^ use of uninitialized variable 'uninit'
}

void direct_copy_to_buffer()
{
    float uninit;
    output[1] = uninit;
//CHK:        ^ use of uninitialized variable
//CHK:        ^ use of uninitialized variable 'uninit'
}

// Case 2: a direct copy of an uninitialized scalar local into an `out`
// parameter and into an `inout` parameter.
void copy_to_out_param(out float o)
{
    float uninit;
    o = uninit;
//CHK:^ use of uninitialized variable
//CHK:^ use of uninitialized variable 'uninit'
}

void copy_to_inout_param(inout float io)
{
    float uninit;
    io = uninit;
//CHK: ^ use of uninitialized variable
//CHK: ^ use of uninitialized variable 'uninit'
}

// Case 3 (positive control): an uninitialized local used in an expression --
// this was already warned before the fix.
float use_in_expression()
{
    float uninit;
    return uninit + 1.0;
//CHK:            ^ use of uninitialized variable
//CHK:            ^ use of uninitialized variable 'uninit'
}

// Case 4: a swizzled store reads its source operand too, so copying an
// uninitialized value into swizzled components is also diagnosed (exercises
// the `SwizzledStore` arm of the store-family classification, not just `store`).
void swizzle_copy()
{
    float2 uninit;
    float4 v = float4(0.0, 0.0, 0.0, 0.0);
    v.xy = uninit;
//CHK:   ^ use of uninitialized variable
//CHK:   ^ use of uninitialized variable 'uninit'
    output[2] = v.x + v.y;
}

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
}
