//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#coopmatmapelementifunc
//META: doc_section_digest=e491a40c4584c9e2ef051e5d42cd61fb8fe95105a8ef0814ea10a93c9c980d08
//META: purpose=`coopVecOuterProductAccumulate` and `coopVecReduceSumAccumulate` surface the documented `CoopVecOuterProductAccumulate` and `CoopVecReduceSumAccumulate` opcodes in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `CoopVecOuterProductAccumulate` (operands `matrixPtr,
// matrixOffset, a, b, memoryLayout, matrixInterpretation, matrixStride`)
// and `CoopVecReduceSumAccumulate` (`bufferPtr, offset, value`) — the
// training-side cooperative-vector opcodes. The portable surface is
// the `coopVecOuterProductAccumulate` / `coopVecReduceSumAccumulate`
// free functions on `CoopVec` values.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWByteAddressBuffer matrixOut;
RWByteAddressBuffer reduceOut;

[numthreads(1, 1, 1)]
void main()
{
    CoopVec<half, 4> vecA;
    CoopVec<half, 8> vecB;
    for (int i = 0; i < vecA.getCount(); ++i)
        vecA[i] = half(i + 1);
    for (int i = 0; i < vecB.getCount(); ++i)
        vecB[i] = half(i + 1);

    coopVecOuterProductAccumulate(
        vecA,
        vecB,
        matrixOut,
        0,
        32,
        CoopVecMatrixLayout.TrainingOptimal,
        CoopVecComponentType.Float16);

    coopVecReduceSumAccumulate(vecA, reduceOut, 0);
}

// IR-LABEL: func %main
// IR-DAG: CoopVecOuterProductAccumulate
// IR-DAG: CoopVecReduceSumAccumulate
