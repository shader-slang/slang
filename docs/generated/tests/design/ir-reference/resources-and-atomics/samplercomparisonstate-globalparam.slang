//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#shader-io
//META: doc_section_digest=212a9e3fd4ca324be91b034872f2b7a9afdb1fe8ac5acfc9f068664d69c3e326
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
