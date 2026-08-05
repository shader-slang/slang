//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#metalpackedvec
//META: doc_section_digest=167b3150c6048e03cd650e8f68570073a489fbe77cab8ec27c015d56de4d32bd
//META: purpose=Expansion: MetalPackedVec generalises over the element type; a uint3 device-buffer element emits as packed_uint3.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the MSL rendering of MetalPackedVec is the generic
// packed_T<N> form, not a float-only special case. A
// StructuredBuffer<uint3> element mirrors the documented float3
// example with a different scalar element type; METAL asserts the
// buffer is declared with packed_uint3 storage.

//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main

StructuredBuffer<uint3> counts;
RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint3 c = counts[tid.x];
    output[tid.x] = c.x + c.y + c.z;
}

// METAL: packed_uint3 device* counts
