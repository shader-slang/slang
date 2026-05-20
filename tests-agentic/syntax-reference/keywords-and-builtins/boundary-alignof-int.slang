//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#expression-keywords
//META: doc_section_digest=44ec80292e79604a0622888241e4998ddb07e1c6a708c0c3c0b8cfa44ba7028e
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
