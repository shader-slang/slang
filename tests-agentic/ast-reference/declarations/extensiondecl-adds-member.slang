//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#extensiondecl
//META: doc_section_digest=0cda32200a5996680b579cdf7db2b997f7ed67ad17396bc603809449a7bab1db
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
