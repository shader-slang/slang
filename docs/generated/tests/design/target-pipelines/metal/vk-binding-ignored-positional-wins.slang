//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=f18a62d46309fae1d33b5fed992f19c2eff74d722d039dfeeeccc0dfdbf5b824
//META: purpose=Boundary: two textures annotated with the same `vk::binding(0,0)` still emit positional `[[texture(0)]]` and `[[texture(1)]]` slots on Metal.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the positional-binding rule with deliberately conflicting
// Vulkan-style annotations: both textures claim set 0 / binding 0, which
// would be a hard collision on SPIR-V. On Metal the annotation carries no
// weight, so the emitter must still hand out distinct slots in declaration
// order. CHECK asserts `texture(0)` and `texture(1)` both appear on the
// kernel signature.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

[vk::binding(0, 0)] Texture2D<float4> tex0;
[vk::binding(0, 0)] Texture2D<float4> tex1;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex0.SampleLevel(samp, float2(tid.xy), 0)
                  + tex1.SampleLevel(samp, float2(tid.xy), 0);
}

// CHECK: texture(0)
// CHECK-SAME: texture(1)
