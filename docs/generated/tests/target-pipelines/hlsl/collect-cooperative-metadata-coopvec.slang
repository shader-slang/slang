//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#collectcooperativemetadata
//META: doc_section_digest=e8f525c4f3b39107c481bf03414c7c2ad6a1c96a430e84aae54c4f8e6a580692
//META: purpose=collectCooperativeMetadata runs when target capabilities imply cooperative_matrix or cooperative_vector; a CoopVec compute kernel under -profile cs_6_9 emits HLSL that retains the cooperative-vector spelling and the byte-address buffer storage backing it.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Phase C row 28 gates `collectCooperativeMetadata` on
// `targetCaps implies cooperative_matrix or cooperative_vector`. HLSL
// exposes cooperative vectors via DXC's `cs_6_9` profile. We pin
// the HLSL emission of a CoopVec compute kernel: the RWByteAddressBuffer
// storage survives, and a cooperative-vector accumulate call site is
// present (Slang lowers CoopVec accumulate via __builtin_VectorAccumulate
// on cs_6_9; we match a substring tolerant of the mangled spelling).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -profile cs_6_9

RWByteAddressBuffer reduceSumOutput;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    CoopVec<half, 4> vec;
    for (int i = 0; i < vec.getCount(); ++i)
        vec[i] = half(i + 1);

    coopVecReduceSumAccumulate(vec, reduceSumOutput, 0);
}

// CHECK: RWByteAddressBuffer
// CHECK: VectorAccumulate
