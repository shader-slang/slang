//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#expression-keywords
//META: doc_section_digest=44ec80292e79604a0622888241e4998ddb07e1c6a708c0c3c0b8cfa44ba7028e
//META: purpose=`none` is the expression keyword for the empty value of `Optional<T>`; `hasValue` discriminates between the empty and present cases.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Construct two `Optional<int>` values, one from `none` and one from
// a plain integer, then read `hasValue` / `value`. The empty case
// resolves to -1; the present case shows the wrapped 42.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<int> a = none;
    Optional<int> b = 42;
    int av = a.hasValue ? a.value : -1;
    int bv = b.hasValue ? b.value : -1;
    // CHECK: a=-1 b=42
    printf("a=%d b=%d\n", av, bv);
}
