//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#literal-forms-vs-token-kinds
//META: doc_section_digest=f5c669bb9740dfa642523af3da2be0ac7b679950ab4f326668ed87a90288d1e6
//META: purpose=`nullptr` is recognized as a `Literal` atom (NullPtrLiteralExpr) usable to initialize and compare a pointer.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `nullptr` literal: a pointer initialized from it
// compares equal to `nullptr`, so both uses must have parsed as the
// null-pointer literal atom. Runs on the CPU target, where pointer
// types are supported.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int* p = nullptr;
    // CHECK: 7
    outputBuffer[0] = (p == nullptr) ? 7 : 0;
}
