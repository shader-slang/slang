// Issue #10526: the opaque-local-variable check fires for several categories of
// opaque handle and for both the `?:` and `if`/`else` selection patterns, and it
// recurses into struct fields. This complements the basic
// `opaque-local-resource-select.slang` test (which covers a single `Texture1D`
// via `?:`).

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage fragment

Texture2D<float> t0, t1;
SamplerState s0, s1;
SubpassInput<float4> sp0, sp1;

// SamplerState selected via `?:`.
[noinline]
SamplerState selSampler(bool b)
{
    return b ? s1 : s0;
}
//CHECK-DAG: opaque-typed value ('SamplerState') cannot be placed in a function-local variable

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
//CHECK-DAG: opaque-typed value ('Texture2D') cannot be placed in a function-local variable

// SubpassInput selected via `?:` (Vulkan-only, fragment-stage opaque handle).
[noinline]
SubpassInput<float4> selSubpass(bool b)
{
    return b ? sp1 : sp0;
}
//CHECK-DAG: opaque-typed value ('SubpassInput') cannot be placed in a function-local variable

[shader("fragment")]
float main(bool b) : SV_Target
{
    float v = t0.Sample(selSampler(b), float2(0, 0));
    v += selIfElse(b).Sample(s0, float2(0, 0));
    v += selSubpass(b).SubpassLoad().x;
    return v;
}
