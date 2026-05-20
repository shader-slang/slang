//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#expression-keywords
//META: doc_section_digest=44ec80292e79604a0622888241e4998ddb07e1c6a708c0c3c0b8cfa44ba7028e
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
