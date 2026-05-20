//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/syntax-reference/grammar.md#disambiguation
//META: doc_section_digest=175f6752fa3c2908ca7bab0b7c0446710aa1ce890a7fcb52b0d819e2a0b08664
//META: purpose=`<` in a body context where the preceding expression does not resolve to a generic parses as the comparison operator.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative side of the `<` disambiguation: in
// `a < b`, both operands are plain int variables, so the parser falls
// back to treating `<` as a less-than comparison.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 3, b = 5;
    bool r = a < b;
    // CHECK: r=1
    printf("r=%d\n", int(r));
}
