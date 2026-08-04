//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv -entry computeMain -stage compute

// Regression test for #10212.
//
// A literal passed to an `inout` parameter inside a `[Differentiable]` entry
// point should be rejected by semantic checking instead of reaching IR lowering.

uint bump(inout uint seed)
{
    seed += 1;
    return seed;
}

[shader("compute")]
[numthreads(1, 1, 1)]
[Differentiable]
void computeMain()
{
    uint value = bump(13);
//diag:               ^^ argument must be l-value
//diag:               ^^ argument passed to parameter '0' must be l-value.
//diag:               ^^ argument was implicitly cast from 'int' to 'uint', and Slang does not support using an implicit cast as an l-value for this usage
}
