//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#literal-forms-vs-token-kinds
//META: doc_section_digest=c7d6780d315f67918c23ddaaf2cd151fb568fe783747ef75230b9c640ed1629f
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
