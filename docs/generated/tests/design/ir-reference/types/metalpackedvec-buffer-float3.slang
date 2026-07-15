//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:21:07Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/ir-reference/types.md#metalpackedvec
//META: doc_section_digest=99021749fc5be40322edc33a942e6cb039cbc9b5719142b23c52923a90003bdd
//META: purpose=A float3 element of a Metal device buffer is rewritten to MetalPackedVec and emitted as MSL packed_float3.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Metal buffer-element-type legalization replaces the padded
// Vec element type of a device buffer with the tightly packed MetalPackedVec
// form. A StructuredBuffer<float3> global is read in the kernel so the
// buffer parameter survives to emit; METAL asserts the MSL declares the
// buffer with packed_float3 storage, which only appears if the element type
// was rewritten to MetalPackedVec(Float, 3).

//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main

StructuredBuffer<float3> positions;
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3 p = positions[tid.x];
    output[tid.x] = p.x + p.y + p.z;
}

// METAL: packed_float3 device* positions
