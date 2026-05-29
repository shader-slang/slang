//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#disambiguation
//META: doc_section_digest=175f6752fa3c2908ca7bab0b7c0446710aa1ce890a7fcb52b0d819e2a0b08664
//META: purpose=`<` is treated as a generic-argument list when followed by `>` and a "generic-follower" token like `(`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the positive side of the `<` disambiguation: in
// `max<int>(1, 2)`, the parser sees `<int>` followed by `(`, which is
// in the generic-followers set, so `<` opens a generic-argument list.

//TEST:INTERPRET(filecheck=CHECK):

T myMax<T : __BuiltinArithmeticType>(T a, T b) { return a > b ? a : b; }

void main()
{
    int r = myMax<int>(3, 7);
    // CHECK: r=7
    printf("r=%d\n", r);
}
