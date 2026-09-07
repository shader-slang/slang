//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=fb21c47d3ae888177f02abe81a3052f9206d46318a09404685183a1509335f2b
//META: purpose=`InterfaceDecl` parses as `'interface' IDENT '{' InterfaceMember* '}'` and accepts a FuncDecl member.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the InterfaceDecl production with a FuncDecl member as the
// `InterfaceMember`. A struct conforms to the interface and we observe
// the call resolves to its implementation.

//TEST:INTERPRET(filecheck=CHECK):

interface IPing
{
    int ping();
}

struct A : IPing
{
    int ping() { return 5; }
}

void main()
{
    A a;
    // CHECK: ping=5
    printf("ping=%d\n", a.ping());
}
