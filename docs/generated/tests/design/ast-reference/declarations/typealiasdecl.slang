//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
//META: purpose=A TypeAliasDecl introduces a modern-syntax alias (typealias Y = X) for an existing type; the alias is interchangeable with the original.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of TypeAliasDecl. `typealias` is the
// modern-syntax form of `typedef`; the documented behavior is that
// the alias and the original type are interchangeable.

//TEST:INTERPRET(filecheck=CHECK):

typealias Velocity = float;

float scale(float v) { return v * 2.0f; }

void main()
{
    Velocity vel = 1.5f;
    //CHECK: 3.000000
    printf("%f\n", scale(vel));
}
