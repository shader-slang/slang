//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#coopmatmapelementifunc
//META: doc_section_digest=e491a40c4584c9e2ef051e5d42cd61fb8fe95105a8ef0814ea10a93c9c980d08
//META: purpose=`coopMatMulAdd` on three `CoopMat` values surfaces the documented `CoopMatMulAdd` opcode in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's cooperative-matrix-and-vector table lists `CoopMatMulAdd`
// (operands `matA, matB, matC, saturatingAccumulation`) as the IR form
// of the core-module `CoopMat.mulAdd`. The portable surface is the
// `linalg::coopMatMulAdd` free function on `CoopMat<T, ...>` values.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

using namespace linalg;

typealias MatA = CoopMat<uint16_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA>;
typealias MatB = CoopMat<uint16_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixB>;
typealias MatC = CoopMat<uint32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

RWStructuredBuffer<uint32_t> sink;

[numthreads(32, 1, 1)]
void main()
{
    MatA a = MatA(2);
    MatB b = MatB(3);
    MatC c = MatC(4);
    MatC result = coopMatMulAdd<uint32_t, false>(a, b, c);
    result.Store<CoopMatMatrixLayout.RowMajor>(sink, 0, 16);
}

// IR-LABEL: func %main
// IR: CoopMatMulAdd
