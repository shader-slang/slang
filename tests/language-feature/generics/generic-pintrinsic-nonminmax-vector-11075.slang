//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -entry test -stage compute

// The only vector-arity `$P` prelude helpers are for `min`/`max`. Any other `$P` operation reaching
// the expander with a whole-vector operand (here a custom `$P_exp` intrinsic specialized on `float2`
// via an `IFloat`-constrained generic) has only a scalar `<prefix>_exp` helper, so it must be a
// clean language error (E55215), not an unresolvable `F32_exp(vector<float,2>)` call. Guards the
// operation-awareness of the `$P` vector-helper check. Regression test for shader-slang/slang#11075.

__generic<T : IFloat>
[__readNone]
T myExp(T x)
{
    __target_switch
    {
    case cpp:
        __intrinsic_asm "$P_exp($0)";
    case cuda:
        __intrinsic_asm "$P_exp($0)";
    }
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void test()
{
    float2 v = float2(1.0, 2.0);
    float2 r = myExp(v);
    outputBuffer[0] = r.x;
    //CHECK: E55215
    //CHECK: does not support operand type
}
