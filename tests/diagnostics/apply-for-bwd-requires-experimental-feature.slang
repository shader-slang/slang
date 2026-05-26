//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -stage compute -entry computeMain

float foo(float x)
{
    return x;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    __apply(foo);
/*CHECK:
    ^^^^^^^ __apply is experimental and requires '-experimental-feature'
    ^^^^^^^ __apply expressions are rejected unless '-experimental-feature' is enabled.
*/
}
