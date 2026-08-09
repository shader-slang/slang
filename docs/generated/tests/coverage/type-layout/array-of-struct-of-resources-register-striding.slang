//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a bounded array of a struct containing resources is laid out array-of-structs to structs-of-arrays, striding each field's register index by the array element count.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `maybeAdjustLayoutForArrayElementType` has a `StructTypeLayout` arm that
// rebuilds a struct layout when the struct is used as an array element and one
// of its fields consumes a register kind that needs AoS-to-SoA adjustment. For
// a finite element count it multiplies each such field's register index by the
// element count (`resInfo.index *= elementCount`). This test makes that
// arithmetic visible: `Pair` has two `Texture2D` fields, so with four elements
// field `a` keeps `t0` while field `b` moves from `t1` to `t4` — the stride is
// the only thing that can produce a gap of exactly four `t` registers.
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

struct Pair
{
    Texture2D a;
    Texture2D b;
    SamplerState s;
}
Pair pairs[4];
RWStructuredBuffer<float4> outp;
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = pairs[1].a.SampleLevel(pairs[1].s, float2(0, 0), 0) +
              pairs[2].b.SampleLevel(pairs[2].s, float2(0, 0), 0);
}

// Each field becomes its own flat array, and `b` starts one full stride after `a`.
//HLSL-DAG: Texture2D<float4 {{.*}}pairs_a{{[A-Za-z0-9_]*}}[int(4)] : register(t0)
//HLSL-DAG: Texture2D<float4 {{.*}}pairs_b{{[A-Za-z0-9_]*}}[int(4)] : register(t4)
//HLSL-DAG: SamplerState{{.*}}pairs_s{{[A-Za-z0-9_]*}}[int(4)] : register(s0)
