//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#type-checking
//META: doc_section_digest=4f712396b718dc4245b0d7a122fe7c024d74b3b4a70dd75948a651b3aa074c9c
//META: purpose=A generic body is type-checked before specialization using only the declared constraints, so an operation valid for all conforming types compiles once and works for every specialization.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies before-specialization checking: `sq<T : IArithmetic>` uses `a * a`,
// which is legal for every IArithmetic conformer, so the body checks once
// against the constraint (not per instantiation). Two distinct specializations
// int and float both reuse that checked body; CHECK asserts 36 and 2.25,
// proving the single pre-specialization check served both concrete types.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

T sq<T : IArithmetic>(T a)
{
    return a * a;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = float(sq(6));
    outputBuffer[1] = sq(1.5f);
}
// CHECK: 36.0
// CHECK: 2.25
