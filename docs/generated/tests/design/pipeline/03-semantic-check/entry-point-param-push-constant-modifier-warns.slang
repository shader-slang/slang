//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=c00647cfad21101d2bd2d831fcc3ac751988a6256cea5bd14c17a5aeb428a89e
//META: purpose=A `[[vk::push_constant]]` attribute on an entry-point parameter is reported as an unsupported-and-ignored modifier (E38010) on every target.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a second unconditional case of the same check, and specifically on
// the SPIR-V target where the attribute would otherwise look meaningful. The
// diagnostic fires regardless of target because this modifier is never honored
// in this position. CHECK pins E38010 and the attribute name in the message.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -emit-spirv-directly

struct Pc
{
    int x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main([[vk::push_constant]] uniform ConstantBuffer<Pc> pc, uniform RWStructuredBuffer<int> buf)
//CHECK: E38010
//CHECK: attribute '[[vk::push_constant]]' on parameter 'pc'
{
    buf[0] = pc.x;
}
