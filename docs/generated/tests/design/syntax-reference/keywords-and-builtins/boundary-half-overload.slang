//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=950d7a7efcea5da748ec49fa6499555097f3093abb07ac97945a51be1ce2339c
//META: purpose=Boundary: `half` is the narrow floating-point type supplied by `core.meta.slang`; it is distinguishable from `float` by overload resolution.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the lower precision floating-point type. The `half` overload
// must win when the argument is explicitly `half`-typed.

//TEST:INTERPRET(filecheck=CHECK):

int kind(half v)  { return 1; }
int kind(float v) { return 2; }

void main()
{
    half h = half(1.0);
    // CHECK: k=1
    printf("k=%d\n", kind(h));
}
