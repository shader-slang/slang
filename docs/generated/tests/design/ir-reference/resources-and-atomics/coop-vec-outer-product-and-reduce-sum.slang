//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#cooperative-matrix-and-vector
//META: doc_section_digest=5d8bfb7cd06d16d7e5526799f2be56c9cee4a73edf35171033b8a239608e92f4
//META: purpose=`coopVecOuterProductAccumulate` and `coopVecReduceSumAccumulate` surface the documented `CoopVecOuterProductAccumulate` and `CoopVecReduceSumAccumulate` opcodes in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two accumulate-into-memory cooperative-vector opcodes.
// Both write through a buffer pointer, so neither result is dead; the
// CHECKs are order-independent because the two calls are independent
// and the lowering is free to interleave them.

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
