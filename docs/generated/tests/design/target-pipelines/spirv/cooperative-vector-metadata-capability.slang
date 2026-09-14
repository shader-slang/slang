//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=61b9f1fa9e25e0a1a331e30835332c142e930cea0b3de2b952f901beda9edd64
//META: purpose=collectCooperativeMetadata fires when the program uses a cooperative-vector type — the SPIR-V emit declares OpCapability CooperativeVectorNV and routes coopVecLoad/coopVecMatMul through cooperative-vector opcodes.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: collectCooperativeMetadata (Phase C, row 38) is gated
// on targetCaps implying cooperative_matrix or cooperative_vector.
// Using a CoopVec<T,N> in compute pulls the cooperative-vector
// capability set into the SPIR-V emit. The metadata pass captures
// cooperative types that survive lowering so the emit declares the
// right capabilities and routes the loads/matmuls through the
// cooperative-vector opcodes.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int32_t> outBuf;
ByteAddressBuffer input;
ByteAddressBuffer matrix;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    CoopVec<int8_t, 4> vec = coopVecLoad<4, int8_t>(input);

    let result = coopVecMatMul<int32_t, 4, 4>(
        vec,
        CoopVecComponentType::SignedInt8,
        matrix,
        0,
        CoopVecComponentType::SignedInt8,
        CoopVecMatrixLayout::RowMajor,
        false,
        4);

    outBuf[0] = result[0];
}

// CHECK-DAG: OpCapability CooperativeVectorNV
// CHECK-DAG: OpCooperativeVectorMatrixMulNV
