// Metal only accepts `[[stage_in]]` on an entry-point parameter, so a varying input must not
// leak that attribute onto an ordinary function that was derived from an entry point (#13271).
// Each check requires every `stage_in` in the output to sit on a `[[vertex]]`/`[[fragment]]`
// signature line.

//TEST:SIMPLE(filecheck=VS): -target metal -stage vertex -entry vsMain
//TEST:SIMPLE(filecheck=FS): -target metal -stage fragment -entry fsMain
//TEST:SIMPLE(filecheck=EP): -target metal -entry vsA -entry vsB

struct VOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

// A struct-returning vertex shader with an entry-point `uniform`.
//VS-NOT: stage_in
//VS: {{\[\[}}vertex{{\]\]}}{{.*}} vsMain({{.*}}{{\[\[}}stage_in{{\]\]}}
//VS-NOT: stage_in
[shader("vertex")]
VOut vsMain(float3 p : POSITION, uniform float4x4 m)
{
    VOut o;
    o.pos = mul(m, float4(p, 1));
    o.uv = p.xy;
    return o;
}

// A fragment shader with an `out` parameter and an entry-point `uniform`.
//FS-NOT: stage_in
//FS: {{\[\[}}fragment{{\]\]}}{{.*}} fsMain({{.*}}{{\[\[}}stage_in{{\]\]}}
//FS-NOT: stage_in
[shader("fragment")]
void fsMain(float4 c : COLOR, uniform float4 tint, out float4 color : SV_Target)
{
    color = c * tint;
}

// A vertex shader that calls another vertex shader; the callee is also compiled as an ordinary
// function, whose signature must carry no attributes at all.
//EP-NOT: stage_in
//EP: {{^[^[]*}} vsB_0({{[^[]*}}){{$}}
//EP-NOT: stage_in
//EP: {{\[\[}}vertex{{\]\]}}{{.*}}{{\[\[}}stage_in{{\]\]}}
//EP-NOT: stage_in
//EP: {{\[\[}}vertex{{\]\]}}{{.*}}{{\[\[}}stage_in{{\]\]}}
//EP-NOT: stage_in
[shader("vertex")]
VOut vsB(float3 p : POSITION)
{
    VOut o;
    o.pos = float4(p, 1);
    o.uv = p.xy;
    return o;
}

[shader("vertex")]
VOut vsA(float3 p : POSITION)
{
    VOut o = vsB(p);
    o.uv = o.uv * 2;
    return o;
}
