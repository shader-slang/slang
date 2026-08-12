//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=2b1e19c46f66c272b88b6086898b21a84533837b5f2bf1884cde7b0ab677590a
//META: purpose=A ConstructorDecl is spelled `__init`; calling the type as a function dispatches to it and initializes the value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `__init` declares a constructor reachable through
// constructor-call syntax. The constructor multiplies its argument before
// storing it, so the CHECK's expected value can only be produced by the
// constructor body running rather than by field-wise initialization.

//TEST:INTERPRET(filecheck=CHECK):

struct Wrap
{
    int v;
    __init(int x) { v = x * 3; }
}

void main()
{
    Wrap w = Wrap(5);
    //CHECK: 15
    printf("%d\n", w.v);
}
