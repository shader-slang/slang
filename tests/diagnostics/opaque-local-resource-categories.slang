// `specializeResourceUsage` exposes the conditional resources returned by `selSampler` and
// `selIfElse` in `main`. `specializeResourceSelections` then branches around their `Sample` calls,
// so these cases must compile without opaque local storage. Neither pass can legalize the
// `SubpassInput` result of `selSubpass`, which must still reach the opaque-storage diagnostic.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target spirv -entry main -stage fragment

Texture2D<float> t0, t1;
SamplerState s0, s1;
SubpassInput<float4> sp0, sp1;

// SamplerState selected via `?:`.
[noinline]
SamplerState selSampler(bool b)
{
    return b ? s1 : s0;
}

// Texture selected via `if`/`else` (a different post-phi-elimination IR shape
// than `?:`).
[noinline]
Texture2D<float> selIfElse(bool b)
{
    Texture2D<float> r;
    if (b)
        r = t1;
    else
        r = t0;
    return r;
}

// SubpassInput selected via `?:` (Vulkan-only, fragment-stage opaque handle).
[noinline]
SubpassInput<float4> selSubpass(bool b)
{
    return b ? sp1 : sp0;
}
//CHECK-NOT: opaque-typed value ('SamplerState')
//CHECK-NOT: opaque-typed value ('Texture2D')
//CHECK: opaque-typed value ('SubpassInput') cannot be placed in a function-local variable
//CHECK-NOT: opaque-typed value ('SamplerState')
//CHECK-NOT: opaque-typed value ('Texture2D')

[shader("fragment")]
float main(bool b) : SV_Target
{
    float v = t0.Sample(selSampler(b), float2(0, 0));
    v += selIfElse(b).Sample(s0, float2(0, 0));
    v += selSubpass(b).SubpassLoad().x;
    return v;
}
