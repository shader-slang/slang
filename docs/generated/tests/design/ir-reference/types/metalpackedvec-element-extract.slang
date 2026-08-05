//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#metalpackedvec
//META: doc_section_digest=167b3150c6048e03cd650e8f68570073a489fbe77cab8ec27c015d56de4d32bd
//META: purpose=Element extraction from a MetalPackedVec-typed buffer element projects out the scalar element type, as for Vec.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies component extraction still works after the element type of
// a Metal device buffer becomes MetalPackedVec: projecting a component
// must yield the scalar elementType exactly as it does for a plain
// Vec. The kernel stores only the .y component of a packed float3
// element into a scalar output buffer; METAL asserts the packed
// storage type is present and that the emitted kernel performs the .y
// component read (the sole .y access in the program).

//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main

StructuredBuffer<float3> positions;
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    output[tid.x] = positions[tid.x].y;
}

// METAL-DAG: packed_float3 device* positions
// METAL-DAG: .y;
