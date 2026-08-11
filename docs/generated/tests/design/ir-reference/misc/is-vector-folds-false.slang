//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#type-queries-and-predicates
//META: doc_section_digest=dd5dde08daf5ef4afa93de9a5f62e011969dd2709cf191720b25851af591f755
//META: purpose=`__isVector<T>()` folds to `false` for every concrete type, including an actual vector, because the fold unwraps the operand to its element type first.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented quirk of `IsVector` at the one input where it looks
// like it should say yes. The shader asks the predicate about a scalar, a
// two-component vector, a four-component vector and a matrix and sums the
// affirmative answers; the asserted total of 0 shows the same-element-type
// unwrap happens before the predicate for every shape, so no concrete type
// reaches the vector test.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int yes = 0;
    if (__isVector<float>()) yes += 1;
    if (__isVector<float2>()) yes += 1;
    if (__isVector<float4>()) yes += 1;
    if (__isVector<float2x2>()) yes += 1;
    outputBuffer[0] = yes;
}

// CHECK: 0
