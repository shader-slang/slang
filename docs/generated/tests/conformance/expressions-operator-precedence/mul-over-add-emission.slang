//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=CPU compute kernel verifies `*` binds before `+`: buffer reads at runtime defeat constant folding; `a + b * c` with [2,3,4] yields 14.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Compute-dispatch version of the mul-over-add precedence claim.
// Uses a buffer to feed operands at runtime, defeating constant folding,
// so the generated code must actually perform `a + (b * c)` at dispatch.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[2 3 4 0], stride=4)

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int a = outputBuffer[0];
    int b = outputBuffer[1];
    int c = outputBuffer[2];
    outputBuffer[3] = a + b * c;  // must be 2 + (3*4) = 14
}

// CHECK: 2
// CHECK: 3
// CHECK: 4
// CHECK: 14
