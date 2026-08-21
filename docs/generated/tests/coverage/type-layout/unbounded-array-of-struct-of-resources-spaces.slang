//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies an unbounded array of a struct containing resources gives every resource field its own register space, each starting at register index zero.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// This is the infinite-element-count branch of the same struct arm of
// `maybeAdjustLayoutForArrayElementType` that
// `array-of-struct-of-resources-register-striding.slang` covers for a finite
// count. When the element count is unbounded, index striding is impossible, so
// each resource field instead sets `requireNewSpace` and resets its index and
// space to zero, and the enclosing struct hands out one `RegisterSpace` per
// such field. The CHECKs pin exactly that: three fields, three consecutive
// spaces, each field back at register index 0 inside its own space.
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

struct Pair
{
    Texture2D a;
    Texture2D b;
    SamplerState s;
}
Pair pairs[];
RWStructuredBuffer<float4> outp;
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = pairs[1].a.SampleLevel(pairs[1].s, float2(0, 0), 0) +
              pairs[2].b.SampleLevel(pairs[2].s, float2(0, 0), 0);
}

// space0 stays with the ordinary global bindings; the array fields take 1, 2, 3.
//HLSL-DAG: RWStructuredBuffer<float4 {{.*}}outp{{[A-Za-z0-9_]*}} : register(u0)
//HLSL-DAG: Texture2D<float4 {{.*}}pairs_a{{[A-Za-z0-9_]*}}[] : register(t0, space1)
//HLSL-DAG: Texture2D<float4 {{.*}}pairs_b{{[A-Za-z0-9_]*}}[] : register(t0, space2)
//HLSL-DAG: SamplerState{{.*}}pairs_s{{[A-Za-z0-9_]*}}[] : register(s0, space3)
