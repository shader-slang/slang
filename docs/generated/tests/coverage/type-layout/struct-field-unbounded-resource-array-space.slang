//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a struct field that is itself an unbounded resource array is moved into its own sub-element register space while the sibling finite field stays in the parent space.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `StructTypeLayoutBuilder::addField` has an `isInfinite()` branch for a field
// whose register consumption is unbounded: rather than trying to place the
// following fields after it, the builder adds a `SubElementRegisterSpace` to
// the struct and records a `RegisterSpace` entry on the field itself. The
// unsized member must be last, so `s` is declared first; the CHECKs show `s`
// keeping `s0` in the parameter block's own space while the unbounded texture
// array is relocated to `space1` at index 0.
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

struct S
{
    SamplerState s;
    Texture2D t[];
}
ParameterBlock<S> pb;
RWStructuredBuffer<float4> outp;
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = pb.t[3].SampleLevel(pb.s, float2(0, 0), 0);
}

//HLSL-DAG: SamplerState{{.*}}pb_s{{[A-Za-z0-9_]*}} : register(s0)
//HLSL-DAG: Texture2D<float4 {{.*}}pb_t{{[A-Za-z0-9_]*}}[] : register(t0, space1)
