//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a combined `Sampler2D` inside a Metal ParameterBlock takes two argument-buffer element slots and emits as a texture plus sampler pair in the argument-buffer struct.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `MetalArgumentBufferElementLayoutRulesImpl::GetObjectLayout` gives most
// parameter kinds one `MetalArgumentBufferElement`, but has a separate arm
// giving two to the combined kinds (`TextureSampler`,
// `MutableTextureSampler`, `AppendConsumeStructuredBuffer`). A `ParameterBlock`
// is what puts a parameter under the argument-buffer element rules on Metal,
// and `Sampler2D` is the combined spelling; the CHECK pins the two slots
// materializing as two members of the emitted argument-buffer struct, with the
// following `scale` field still present after them.
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute

struct S { Sampler2D combined; float scale; }
ParameterBlock<S> pb;
RWStructuredBuffer<float4> outp;
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = pb.combined.SampleLevel(float2(0, 0), 0) * pb.scale;
}

//METAL: struct Sampler2D{{[A-Za-z0-9_]*$}}
//METAL: texture2d<float, access::sample> texture{{[A-Za-z0-9_]*}};
//METAL: sampler sampler{{[A-Za-z0-9_]*}};
//METAL: struct S{{[A-Za-z0-9_]*$}}
//METAL: Sampler2D{{[A-Za-z0-9_]*}} combined{{[A-Za-z0-9_]*}};
//METAL: float scale{{[A-Za-z0-9_]*}};
