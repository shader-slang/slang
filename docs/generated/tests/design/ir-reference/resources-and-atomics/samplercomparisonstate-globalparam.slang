//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=a773e5d9549bbb68f3cb1b14f6a75cb5ac368be4037f672f043b86eb02481ba9
//META: purpose=A module-scope `SamplerComparisonState` parameter surfaces as a `global_param` with result type `SamplerComparisonState`, distinct from the plain `SamplerState` resource type.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `SamplerComparisonState` carries a distinct IR resource type from
// the plain `SamplerState` (see the sampling-and-combined-samplers
// table). Both share the `global_param` opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform Texture2D<float> tex;
uniform SamplerComparisonState sampCmp;
uniform RWStructuredBuffer<float> outbuf;

[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outbuf[0] = tex.SampleCmp(sampCmp, float2(0, 0), 0.5);
}

// IR: %sampCmp{{.*}}: SamplerComparisonState{{.*}}= global_param
