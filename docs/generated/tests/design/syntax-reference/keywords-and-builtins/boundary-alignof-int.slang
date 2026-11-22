//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#expression-keywords
//META: doc_section_digest=dfed8ec8651a096c4ad6a7aed9f4bc652318061d4ce6e334e3058975c328e942
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
