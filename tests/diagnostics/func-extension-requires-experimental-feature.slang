//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

float foo(float x)
{
    return x;
}

__func_extension fwd_diff(foo)(DifferentialPair<float> x) -> DifferentialPair<float>
//CHECK:         ^^^^^^^^ __func_extension is experimental and requires '-experimental-feature'
//CHECK:         ^^^^^^^^ __func_extension declarations are rejected unless '-experimental-feature' is enabled.
{
    return x;
}
