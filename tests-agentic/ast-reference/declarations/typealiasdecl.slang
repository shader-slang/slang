//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
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
