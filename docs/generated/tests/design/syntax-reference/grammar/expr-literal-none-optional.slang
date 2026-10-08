//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#literal-forms-vs-token-kinds
//META: doc_section_digest=f5c669bb9740dfa642523af3da2be0ac7b679950ab4f326668ed87a90288d1e6
//META: purpose=`none` is recognized as a `Literal` atom (NoneLiteralExpr) usable to initialize an Optional value.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `none` literal: initializing an Optional<int> with it
// yields an absent value (hasValue false), and reassigning a real
// value flips it — so `none` must have parsed as the none-literal
// atom, not as a name lookup.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Optional<int> o = none;
    // CHECK: 0
    outputBuffer[0] = o.hasValue ? 1 : 0;
    o = 5;
    // CHECK: 5
    outputBuffer[1] = o.hasValue ? o.value : -1;
}
