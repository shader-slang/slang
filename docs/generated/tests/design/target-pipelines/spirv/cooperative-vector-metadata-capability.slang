//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
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
