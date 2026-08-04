//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#expression-keywords
//META: doc_section_digest=5cf1a0be2720fdb7640ba4534ce97ec9428cc65b37576f2b89297596bf873eb6
//META: purpose=Boundary: `countof` is documented in the expression-keywords group with `sizeof`/`alignof`; for a variadic value pack of three elements, `countof(D)` is 3.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Exercises the third member of the `sizeof`/`alignof`/`countof`
// triad. `countof` is most cleanly observable on a variadic value
// pack — array-typed `countof` is not constant-folded in the
// interpreter for plain locals, so the pack form is the reliable
// probe of the keyword spelling.

//TEST:INTERPRET(filecheck=CHECK):

struct Dims<let each D : int>
{
    static const int dimCount = countof(D);
}

void main()
{
    // CHECK: c=3
    printf("c=%d\n", Dims<10, 20, 30>.dimCount);
}
