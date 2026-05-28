//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-supplied-vocabulary
//META: doc_section_digest=a84f2b8e553f246fd0e2611d7bd357de1ceb312fa259d862c4056388b6d6a68a
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
