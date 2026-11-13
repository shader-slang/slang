//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#punctuation-and-structural-symbols
//META: doc_section_digest=0786d0795f8ef21f235cb47bedc955b7c43f8ba5c9f9c5989e209bfdddf3d063
//META: purpose=The two-character spelling `::` lexes as a single `Scope` token that separates a namespace from the name it qualifies.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `::` is one token rather than two adjacent `Colon` tokens. A pair of
// colons would parse as an empty label / type-annotation and the call would not
// resolve, so successfully calling through the qualified name and printing the
// callee's return value is what proves the single-token lexing.

//TEST:INTERPRET(filecheck=CHECK):

namespace Outer
{
    int seven() { return 7; }
}

void main()
{
    // CHECK: scope=7
    printf("scope=%d\n", Outer::seven());
}
