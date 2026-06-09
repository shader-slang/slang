//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-supplied-vocabulary
//META: doc_section_digest=a84f2b8e553f246fd0e2611d7bd357de1ceb312fa259d862c4056388b6d6a68a
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
