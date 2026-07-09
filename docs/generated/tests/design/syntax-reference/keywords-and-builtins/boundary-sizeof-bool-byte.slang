//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#expression-keywords
//META: doc_section_digest=5cf1a0be2720fdb7640ba4534ce97ec9428cc65b37576f2b89297596bf873eb6
//META: purpose=Boundary: `sizeof(bool)` resolves to 1 byte; probes the lower end of the scalar-size domain.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pairs with `boundary-sizeof-double.slang` to bracket the documented
// scalar-size domain on the small side. `bool` is the narrowest
// core-module scalar.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: s=1
    printf("s=%d\n", int(sizeof(bool)));
}
