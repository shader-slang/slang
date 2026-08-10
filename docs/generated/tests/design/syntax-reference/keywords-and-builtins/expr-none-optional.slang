//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#expression-keywords
//META: doc_section_digest=6c39a8a833fc084f0b1f7e30400e6efc997d8e919471186717dfce4ca0bf8e11
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
