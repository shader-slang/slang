//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#type-defining-declarations
//META: doc_section_digest=49a7ad484e2dfba8ca655298b70ff54f8a19782584f47ca1a503b190c156a5cf
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
