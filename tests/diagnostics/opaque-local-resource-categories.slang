// Texture and sampler selections returned by helpers can be legalized. SubpassInput resource
// outputs are not supported by resource-output specialization, so their opaque storage must
// still be diagnosed. The texture and sampler cases guard against restoring the old rejection.

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
