//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#cooperative-matrix-and-vector
//META: doc_section_digest=5d8bfb7cd06d16d7e5526799f2be56c9cee4a73edf35171033b8a239608e92f4
//META: purpose=`coopMatMulAdd` on three `CoopMat` values surfaces the documented `CoopMatMulAdd` opcode in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a cooperative-matrix fused multiply-add is one opcode
// rather than a multiply followed by an add. The three matrices come
// from loads so nothing folds, and the CHECK finds the opcode in the
// user body.

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
