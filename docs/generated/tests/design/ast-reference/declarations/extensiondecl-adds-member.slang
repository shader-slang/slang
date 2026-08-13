//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#extensiondecl
//META: doc_section_digest=ad88a19c43c96571c45e942725f5abaf6372424eba2bc248fca5d077dd824f5c
//META: purpose=An ExtensionDecl attaches new members to an existing type; the added member is callable on values of that type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that members written inside `extension T { ... }` become members of
// `T` after name lookup resolves the extension's target type. The struct
// itself declares no method; the CHECK asserts the extension method's result,
// which is only reachable if the attachment happened.

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
