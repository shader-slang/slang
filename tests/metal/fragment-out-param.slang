// Regression test for #11969: a fragment entry point that returns its color through an `out`/`inout`
// parameter (rather than a return value) used to crash the Metal backend with
// "Unknown addressspace encountered." The out-param -> return-struct lowering that already runs for
// vertex shaders now also runs for fragment shaders, so the `out` param becomes a `[[color(N)]]`
// field of a fragment output struct instead of surviving as a pointer-typed parameter into emit.

//TEST:SIMPLE(filecheck=OUT): -target metal -stage fragment -entry fragmentOut
//TEST:SIMPLE(filecheck=OUTIDX): -target metal -stage fragment -entry fragmentOutIndexed
//TEST:SIMPLE(filecheck=INOUT): -target metal -stage fragment -entry fragmentInOut
//TEST:SIMPLE(filecheck=WITHIN): -target metal -stage fragment -entry fragmentOutWithInput
//TEST:SIMPLE(filecheck=MRT): -target metal -stage fragment -entry fragmentMRT
//TEST:SIMPLE(filecheck=BYVAL): -target metal -stage fragment -entry fragmentByValStruct
//TEST:SIMPLE(filecheck=MIXED): -target metal -stage fragment -entry fragmentMixedStructAndOut
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage fragment -entry fragmentOut

// OUT: {{\[\[}}color(0){{\]\]}}
void fragmentOut(out float4 color_out : SV_Target)
{
    color_out = float4(1, 0, 0, 1);
}

// OUTIDX: {{\[\[}}color(0){{\]\]}}
void fragmentOutIndexed(out uint4 color_out : SV_Target0)
{
    color_out = uint4(1, 2, 3, 4);
}

// INOUT: {{\[\[}}color(0){{\]\]}}
void fragmentInOut(inout float4 color_out : SV_Target)
{
    color_out.x = 1;
}

// A varying input alongside the out param must both be handled.
// WITHIN: {{\[\[}}color(0){{\]\]}}
void fragmentOutWithInput(float4 uv : TEXCOORD0, out float4 color_out : SV_Target)
{
    color_out = uv;
}

// Multiple render targets via multiple out params.
// MRT-DAG: {{\[\[}}color(0){{\]\]}}
// MRT-DAG: {{\[\[}}color(1){{\]\]}}
void fragmentMRT(out float4 a : SV_Target0, out float4 b : SV_Target1)
{
    a = float4(1, 0, 0, 1);
    b = float4(0, 1, 0, 1);
}

// A fragment that returns its outputs through a by-value struct is already handled downstream and
// must NOT be re-wrapped by this lowering (that would strip the field semantics). This pins the
// stage-gated struct-return trigger: the color/depth attributes survive unchanged.
struct FragOut
{
    float4 color : SV_Target;
    float depth : SV_Depth;
}
// BYVAL-DAG: {{\[\[}}color(0){{\]\]}}
// BYVAL-DAG: {{\[\[}}depth(any){{\]\]}}
FragOut fragmentByValStruct()
{
    FragOut o;
    o.color = float4(1, 0, 0, 1);
    o.depth = 0.5;
    return o;
}

// A fragment that MIXES a by-value struct return with an extra `out` render target. The out param
// forces the lowering (it must, on either stage), and the pre-existing struct-return fields must
// still reach their attributes: color(0)/depth(any) from FragOut plus color(1) from the out param.
// MIXED-DAG: {{\[\[}}color(0){{\]\]}}
// MIXED-DAG: {{\[\[}}depth(any){{\]\]}}
// MIXED-DAG: {{\[\[}}color(1){{\]\]}}
FragOut fragmentMixedStructAndOut(out float4 extra : SV_Target1)
{
    FragOut o;
    o.color = float4(1, 0, 0, 1);
    o.depth = 0.5;
    extra = float4(0, 1, 0, 1);
    return o;
}

// METALLIB: define {{.*}} @fragmentOut
