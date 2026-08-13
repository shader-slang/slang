//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#expression-keywords
//META: doc_section_digest=6c39a8a833fc084f0b1f7e30400e6efc997d8e919471186717dfce4ca0bf8e11
//META: purpose=Boundary: `alignof` is documented in the same expression-keyword group as `sizeof`; `alignof(int)` returns 4, matching the size for a naturally aligned scalar.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `alignof` is listed in the expression-keywords table next to
// `sizeof` and `countof`. This test exercises the spelling and
// confirms it returns the natural alignment of `int`.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: a=4
    printf("a=%d\n", int(alignof(int)));
}
