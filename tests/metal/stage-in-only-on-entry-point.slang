// Metal only accepts `[[stage_in]]` on an entry-point parameter, so a varying input must not
// leak that attribute onto an ordinary function that was derived from an entry point (#13271).
// Each text check requires every `stage_in` in the output to sit on a `[[vertex]]`/`[[fragment]]`
// signature line; the metallib checks confirm the Metal compiler accepts the output.

//TEST:SIMPLE(filecheck=VS): -target metal -stage vertex -entry vsMain
//TEST:SIMPLE(filecheck=FS): -target metal -stage fragment -entry fsMain
//TEST:SIMPLE(filecheck=EP): -target metal -entry vsA -entry vsB
//TEST:SIMPLE(filecheck=VSLIB): -target metallib -stage vertex -entry vsMain
//TEST:SIMPLE(filecheck=FSLIB): -target metallib -stage fragment -entry fsMain
//TEST:SIMPLE(filecheck=EPLIB): -target metallib -entry vsA -entry vsB

// VSLIB: @vsMain
// FSLIB: @fsMain
// EPLIB-DAG: @vsA
// EPLIB-DAG: @vsB

struct VOut
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

// The wrapper built by lowerOutParameters for a struct return must absorb the original function.
// The entry-point `uniform` is essential: its hoisted parameter used to keep the original alive.
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

// The same wrapper, built here for an `out` parameter; the `uniform` is again essential.
//FS-NOT: stage_in
//FS: {{\[\[}}fragment{{\]\]}}{{.*}} fsMain({{.*}}{{\[\[}}stage_in{{\]\]}}
//FS-NOT: stage_in
[shader("fragment")]
void fsMain(float4 c : COLOR, uniform float4 tint, out float4 color : SV_Target)
{
    color = c * tint;
}

// A vertex shader that calls another vertex shader: fixEntryPointCallsites compiles the callee
// as an ordinary function too. The `vsB_0` check matches that copy's one-line signature, which
// must contain no `[` at all and end at its `)`, so a call site (ending in `;`) cannot match. The
// two `[[vertex]]` checks match the vsA and vsB entry points, in either order.
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
