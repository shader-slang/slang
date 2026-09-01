// Negative regression for https://github.com/shader-slang/slang/issues/11730.
//
// The fix for #11730 only reports an initializer-list argument as a viable
// coercion when the corresponding constructor actually type-checks; it does not
// blanket-accept brace initializers. A brace initializer that genuinely cannot
// form the parameter type must still be diagnosed. Here `{a, a, a}` is three
// `float2` values (six scalar components) being passed where a `float3` is
// expected, which over-populates the vector and must error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

float encode3(float3 v)
{
    return v.x;
}

void test()
{
    float2 a = float2(1, 2);
    encode3({a, a, a});
    //CHECK: type mismatch in expression
}
