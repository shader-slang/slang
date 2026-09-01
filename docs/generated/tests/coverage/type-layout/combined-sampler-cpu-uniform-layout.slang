//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a global `Sampler2D` on the CPU target is laid out as two pointer-sized uniform slots and emits as a texture plus sampler pair.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `CPUObjectLayoutRulesImpl::GetObjectLayout` has a `TextureSampler` /
// `MutableTextureSampler` arm that returns an `ObjectLayoutInfo` carrying *two*
// pointer-sized uniform entries instead of one. `Sampler2D` is the combined
// spelling that selects it. The CPU target materializes each uniform entry as a
// field in the global-parameter struct, so the emitted C++ showing
// `Sampler2D_0` as exactly a `Texture2D` handle followed by a `SamplerState`
// handle inside `GlobalParams_0` is the observable form of that two-entry
// layout.
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

Sampler2D combined;
RWStructuredBuffer<float4> outp;
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = combined.SampleLevel(float2(0, 0), 0);
}

//CPP: struct Sampler2D{{[A-Za-z0-9_]*$}}
//CPP: Texture2D<Vector<float, 4> > texture{{[A-Za-z0-9_]*}};
//CPP: SamplerState sampler{{[A-Za-z0-9_]*}};
//CPP: Sampler2D{{[A-Za-z0-9_]*}} combined{{[A-Za-z0-9_]*}};
