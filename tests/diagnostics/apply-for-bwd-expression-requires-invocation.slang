//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -stage compute -entry computeMain

float foo(float x)
{
    return x;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    let f = __apply(foo);
//CHECK:    ^^^^^^^ __apply must be invoked directly
//CHECK:    ^^^^^^^ __apply expressions must be invoked directly so they can resolve to apply_bwd.
}
