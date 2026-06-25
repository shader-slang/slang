//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-supplied-vocabulary
//META: doc_section_digest=a84f2b8e553f246fd0e2611d7bd357de1ceb312fa259d862c4056388b6d6a68a
//META: purpose=Boundary: `int64_t` from `core.meta.slang` at its MAX value (9223372036854775807) binds the `int64_t` overload over a competing `int` overload.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the upper edge of the wide signed integer type contributed
// by the core meta-module. The value exceeds 32-bit range, so an
// `int` overload would lose.

//TEST:INTERPRET(filecheck=CHECK):

int kind(int64_t v) { return 1; }
int kind(int v)     { return 2; }

void main()
{
    int64_t x = 9223372036854775807;
    // CHECK: k=1
    printf("k=%d\n", kind(x));
}
