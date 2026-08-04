// A `__ref String` parameter inlines even when the callee declines inlining.
//
// `TypeInliningPass` is what reduces `getStringHash(s)` to the hash of a string *literal*, so a
// surviving call would leave a non-literal operand and trip the `kIROp_StringLit` assert in
// `getStringSlice`. `[noinline]` may decline force-inlining for an ordinary `__ref` parameter, but
// not for a referenced type that requires it.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<uint> buf;

[noinline]
void readHash(__ref String s, __ref uint result)
{
    result = getStringHash(s);
}

//HLSL: buf_0[int(0)] = uint(684824882);
//HLSL-NOT: readHash

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    String s = "hello";
    uint r = 0;
    readHash(s, r);
    buf[0] = r;
}
