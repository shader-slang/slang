//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive):

float foo(float x)
{
    return x;
}

__func_extension __apply(foo)(float x) -> float
{
    return x;
}

// CHECK: invalid __func_extension __apply return type
// CHECK: return type of __func_extension __apply must be Tuple<RetType, CtxType>.
