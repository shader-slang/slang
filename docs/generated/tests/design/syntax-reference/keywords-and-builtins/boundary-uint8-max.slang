//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=950d7a7efcea5da748ec49fa6499555097f3093abb07ac97945a51be1ce2339c
//META: purpose=Boundary: `uint8_t` from `core.meta.slang` at its MAX value (255) binds the `uint8_t` overload and stores the documented max value.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the upper edge of the narrow unsigned integer type
// contributed by the core meta-module.

//TEST:INTERPRET(filecheck=CHECK):

int kind(uint8_t v) { return 1; }
int kind(uint v)    { return 2; }

void main()
{
    uint8_t x = 255u;
    // CHECK: k=1 v=255
    printf("k=%d v=%d\n", kind(x), int(x));
}
