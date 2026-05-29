//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#coopmatmapelementifunc
//META: doc_section_digest=e491a40c4584c9e2ef051e5d42cd61fb8fe95105a8ef0814ea10a93c9c980d08
//META: purpose=`CoopMat.MapElement` invoked with an element-wise function surfaces the documented `CoopMatMapElementIFunc` opcode in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `CoopMatMapElementIFunc` (variadic, `min=2`) as the IR
// form of the core-module `CoopMat.mapElement`. The portable surface
// uses `MapElement` on a tuple of matrices with a per-element callable.

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
