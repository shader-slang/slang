//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#expression-keywords
//META: doc_section_digest=13c9d348ad0da645a7829d3d113c15ae4df72fa2a418b0b196c8b655d8ea1d92
//META: purpose=`true` and `false` are expression keywords producing boolean-typed values; they select different branches of a conditional.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Assign `true` and `false` to `bool` locals and use them as the
// conditional in a ternary. The two arms emit distinct values so a
// keyword-stripping bug would not be silently masked.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    bool t = true;
    bool f = false;
    int ti = t ? 7 : 9;
    int fi = f ? 7 : 9;
    // CHECK: t=7 f=9
    printf("t=%d f=%d\n", ti, fi);
}
