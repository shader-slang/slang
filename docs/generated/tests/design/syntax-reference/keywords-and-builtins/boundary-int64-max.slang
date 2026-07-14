//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=950d7a7efcea5da748ec49fa6499555097f3093abb07ac97945a51be1ce2339c
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
