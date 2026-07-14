//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:21:07Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/ir-reference/types.md#metalpackedvec
//META: doc_section_digest=99021749fc5be40322edc33a942e6cb039cbc9b5719142b23c52923a90003bdd
//META: purpose=Expansion: MetalPackedVec generalises over the element type; a uint3 device-buffer element emits as packed_uint3.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the MSL rendering of MetalPackedVec is the generic packed_T<N>
// form, not a float-only special case. A StructuredBuffer<uint3> element
// mirrors the documented float3 example with a different scalar element
// type; METAL asserts the buffer is declared with packed_uint3 storage.

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
