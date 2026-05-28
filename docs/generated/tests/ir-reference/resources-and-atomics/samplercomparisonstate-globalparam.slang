//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:40:00+00:00
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=21e167102f1daeca19e02f308c112de6dba84ede8a917f058d59ae80138afd50
//META: purpose=A module-scope `SamplerComparisonState` parameter surfaces as a `global_param` with result type `SamplerComparisonState`, distinct from the plain `SamplerState` resource type.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `SamplerComparisonState` carries a distinct IR resource type from
// the plain `SamplerState` (see the sampling-and-combined-samplers
// table). Both share the `global_param` opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform Texture2D<float> tex;
uniform SamplerComparisonState sampCmp;
uniform RWStructuredBuffer<float> outbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outbuf[0] = tex.SampleCmp(sampCmp, float2(0, 0), 0.5);
}

// IR: %sampCmp{{.*}}: SamplerComparisonState{{.*}}= global_param
