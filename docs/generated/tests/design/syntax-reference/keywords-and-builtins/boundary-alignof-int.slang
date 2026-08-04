//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#expression-keywords
//META: doc_section_digest=5cf1a0be2720fdb7640ba4534ce97ec9428cc65b37576f2b89297596bf873eb6
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
