//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#expression-keywords
//META: doc_section_digest=44ec80292e79604a0622888241e4998ddb07e1c6a708c0c3c0b8cfa44ba7028e
//META: purpose=Boundary: the `none` expression-keyword constructs an empty `Optional<T>` for `T = float` (the smallest floating-point payload), complementing the `Optional<int>` smoke test.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pairs with `expr-none-optional.slang`, which uses `Optional<int>`.
// This one switches the payload type to `float` to probe `none`
// against the floating-point arm of the core-module vocabulary.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<float> a = none;
    Optional<float> b = 3.0f;
    float av = a.hasValue ? a.value : -1.0f;
    float bv = b.hasValue ? b.value : -1.0f;
    // CHECK: a=-1 b=3
    printf("a=%d b=%d\n", int(av), int(bv));
}
