//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#literal-forms-vs-token-kinds
//META: doc_section_digest=c7d6780d315f67918c23ddaaf2cd151fb568fe783747ef75230b9c640ed1629f
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
