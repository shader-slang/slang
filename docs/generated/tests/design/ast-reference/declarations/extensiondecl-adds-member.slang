//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#extensiondecl
//META: doc_section_digest=05e6c25d2d157e10628b6b3d7185f1f92df372676d332ad265d71884285f0c78
//META: purpose=An ExtensionDecl attaches new members to an existing type; the added member is callable on values of that type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of ExtensionDecl: the member declared
// inside an `extension T { ... }` block is reachable through normal
// member-access on a value of type `T`. If the extension were not
// attached, `doubled()` would not resolve.

//TEST:INTERPRET(filecheck=CHECK):

struct Counter { int v; }

extension Counter
{
    int doubled() { return v * 2; }
}

void main()
{
    Counter c; c.v = 21;
    //CHECK: 42
    printf("%d\n", c.doubled());
}
