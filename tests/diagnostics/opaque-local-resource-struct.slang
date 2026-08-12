// Issue #10526: the opaque-local-variable check recurses into struct fields, so
// selecting a struct that contains a resource through control flow is also
// rejected.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage fragment

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
//CHECK: 56004{{.*}}opaque type in local variable

[shader("fragment")]
float main(bool b) : SV_Target
{
    Pair p = selStruct(b);
    return p.a.Sample(samp, float2(0, 0));
}
