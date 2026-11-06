// Resource-type legalization separates `Pair` into its texture fields. `specializeResourceUsage`
// exposes the fields returned by `selStruct` at its call site. `specializeResourceSelections` must
// then branch to sample `t0` or `t1`, without storing a texture in a function-local variable.
// The implicit sample still warrants a warning about the introduced conditional execution.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment -validate-ir -validate-ir-detailed
//CHECK: warning[E56020]
//CHECK: OpImageSampleImplicitLod
//CHECK: OpImageSampleImplicitLod

Texture2D<float> t0, t1;
SamplerState samp;

struct Pair
{
    Texture2D<float> a;
    Texture2D<float> b;
};

[noinline]
Pair selStruct(bool b)
{
    Pair p0 = {t0, t0};
    Pair p1 = {t1, t1};
    return b ? p1 : p0;
}

[shader("fragment")]
float main(bool b) : SV_Target
{
    Pair p = selStruct(b);
    return p.a.Sample(samp, float2(0, 0));
}
