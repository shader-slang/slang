//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -entry test -stage compute

// The `$P` vector-helper check matches the full `_min(`/`_max(` operation token, not a bare `_min`
// prefix, so an operation whose name merely starts with `min`/`max` (here a custom `$P_minValue`)
// is not mistaken for `min`/`max`: it has no vector helper and must be diagnosed (E55215) on a
// whole-vector operand rather than emitting an unresolvable `F32_minValue(vector<float,2>)` call.
// Guards the `_min(`-token match in the `$P` expander for shader-slang/slang#11075.

__generic<T : IFloat>
[__readNone]
T minValue(T x)
{
    __target_switch
    {
    case cpp:
        __intrinsic_asm "$P_minValue($0)";
    case cuda:
        __intrinsic_asm "$P_minValue($0)";
    }
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void test()
{
    float2 v = float2(1.0, 2.0);
    float2 r = minValue(v);
    outputBuffer[0] = r.x;
    //CHECK: E55215
    //CHECK: does not support operand type
}
