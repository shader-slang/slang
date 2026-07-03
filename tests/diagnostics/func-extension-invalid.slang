//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -experimental-feature

float foo(float x)
{
    return x;
}

struct Ctx
{
    void operator()(out float dx, float dOut)
    {
        dx = dOut;
    }
};

__func_extension sizeof(float)(float x) -> float
{
    return x;
}

// CHECK: unsupported target expression in __func_extension
// CHECK: __func_extension target must be fwd_diff(...), bwd_diff(...), or __apply(...).

[numthreads(1, 1, 1)]
void kernel(uint3 tid: SV_DispatchThreadID)
{
}

__func_extension __dispatch_kernel(kernel, uint3(1, 1, 1), uint3(1, 1, 1))() -> void
{
}

// CHECK: unsupported operator in __func_extension
// CHECK: __func_extension target must use fwd_diff, bwd_diff, or __apply.

__func_extension __apply(foo)(float x) -> float
{
    return x;
}

// CHECK: invalid __func_extension __apply return type
// CHECK: return type of __func_extension __apply must be Tuple<RetType, CtxType>.

__func_extension __apply(foo)(float x) -> Tuple<float, Ctx, int>
{
    return makeTuple(x, Ctx(), 0);
}

// CHECK: invalid __func_extension __apply return type
// CHECK: return type of __func_extension __apply must be Tuple<RetType, CtxType>.

__func_extension fwd_diff((foo))(DifferentialPair<float> x) -> DifferentialPair<float>
{
    return x;
}

// CHECK: could not resolve target function in __func_extension
// CHECK: could not resolve target function in __func_extension.
