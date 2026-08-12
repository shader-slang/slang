//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#cooperative-matrix-and-vector
//META: doc_section_digest=5d8bfb7cd06d16d7e5526799f2be56c9cee4a73edf35171033b8a239608e92f4
//META: purpose=`CoopMat.MapElement` invoked with an element-wise function surfaces the documented `CoopMatMapElementIFunc` opcode in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an element-wise map over a cooperative matrix becomes a
// single opcode carrying the matrix and the callable, rather than a
// lowered loop. The result is stored through a buffer so it survives,
// and the CHECK looks for the opcode inside `func %main`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

using namespace linalg;

typealias MatT = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

StructuredBuffer<float> srcA;
StructuredBuffer<float> srcB;
RWStructuredBuffer<float> sink;

float addPair(uint32_t row, uint32_t col, float a, float b)
{
    return a + b;
}

[numthreads(32, 1, 1)]
void main()
{
    let stride = 16;
    let a = MatT.Load<CoopMatMatrixLayout.RowMajor>(srcA, 0, stride);
    let b = MatT.Load<CoopMatMatrixLayout.RowMajor>(srcB, 0, stride);
    MatT result = makeTuple(a, b).MapElement(addPair);
    result.Store<CoopMatMatrixLayout.RowMajor>(sink, 0, stride);
}

// IR-LABEL: func %main
// IR: CoopMatMapElementIFunc
