//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Regression test for #9700.
//
// Passing a literal constant as an `inout` (or `out`) argument used to
// ICE during IR lowering ("unimplemented: assignment") because the
// front-end's implicit-cast-as-lvalue mechanism was applied even when
// the underlying argument expression was not an l-value. We now reject
// both shapes with an ordinary `argument must be l-value` diagnostic.

float useInout(inout uint seed)
{
    seed += 1;
    return float(seed);
}

void useOut(out uint seed)
{
    seed = 0;
}

int returnInt() { return 7; }

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // Literal r-value
    float bar = useInout(0);
//diag:                  ^ argument must be l-value
//diag:                  ^ argument passed to parameter '0' must be l-value.
//diag:                  ^ argument was implicitly cast from 'int' to 'uint', and Slang does not support using an implicit cast as an l-value for this usage
    useOut(0);
//diag:    ^ argument must be l-value
//diag:    ^ argument passed to parameter '0' must be l-value.
//diag:    ^ argument was implicitly cast from 'int' to 'uint', and Slang does not support using an implicit cast as an l-value for this usage

    // Non-literal r-value (function call result) — same rule should
    // catch it: the inner expression is not an l-value either.
    float baz = useInout(returnInt());
//diag:                           ^ argument must be l-value
//diag:                           ^ argument passed to parameter '0' must be l-value.
//diag:                           ^ argument was implicitly cast from 'int' to 'uint', and Slang does not support using an implicit cast as an l-value for this usage
}
