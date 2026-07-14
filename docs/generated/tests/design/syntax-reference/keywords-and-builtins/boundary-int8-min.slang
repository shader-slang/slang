//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=950d7a7efcea5da748ec49fa6499555097f3093abb07ac97945a51be1ce2339c
//META: purpose=Boundary: `int8_t` is one of the narrow integer types supplied by `core.meta.slang`; the value -128 is its documented MIN and selects the `int8_t` overload via overload resolution.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the lower edge of the narrow signed integer type contributed
// by the core meta-module. If `int8_t` were not in the environment,
// the overload would not compile.

//TEST:INTERPRET(filecheck=CHECK):

int kind(int8_t v) { return 1; }
int kind(int v)    { return 2; }

void main()
{
    int8_t x = -128;
    // CHECK: k=1 v=-128
    printf("k=%d v=%d\n", kind(x), int(x));
}
