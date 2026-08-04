//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#coopmatmapelementifunc
//META: doc_section_digest=e491a40c4584c9e2ef051e5d42cd61fb8fe95105a8ef0814ea10a93c9c980d08
//META: purpose=`coopVecMatMulAdd` on a `CoopVec` with an optional bias surfaces the documented `CoopVecMatMulAdd` opcode in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `CoopVecMatMulAdd` (operands `input, ..., matrixPtr,
// matrixOffset, matrixInterpretation, k, memoryLayout, transpose,
// matrixStride, biasPtr?, biasOffset?, biasInterpretation?`) as the
// IR form of `CoopVec.matMulAdd`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

ByteAddressBuffer input;
ByteAddressBuffer matrix;
ByteAddressBuffer bias;
RWStructuredBuffer<half> sink;

[numthreads(1, 1, 1)]
void main()
{
    CoopVec<half, 4> vec = coopVecLoad<4, half>(input);
    let result = coopVecMatMulAdd<half, 4, 4>(
        vec,
        CoopVecComponentType.Float16,
        matrix,
        0,
        CoopVecComponentType.Float16,
        bias,
        0,
        CoopVecComponentType.Float16,
        CoopVecMatrixLayout.RowMajor,
        false,
        8);

    for (int i = 0; i < result.getCount(); ++i)
        sink[i] = result[i];
}

// IR-LABEL: func %main
// IR: CoopVecMatMulAdd
