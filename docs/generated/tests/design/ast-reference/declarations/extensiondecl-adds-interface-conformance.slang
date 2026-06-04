//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#extensiondecl
//META: doc_section_digest=0cda32200a5996680b579cdf7db2b997f7ed67ad17396bc603809449a7bab1db
//META: purpose=An ExtensionDecl can add interface conformance to an existing type; the type then dispatches through the interface.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an ExtensionDecl can add a new InheritanceDecl to a
// pre-existing AggTypeDecl. The base type does not itself declare
// conformance to `IDouble`, but the extension adds it; the
// interface-constrained generic must accept values of the base type.

//TEST:INTERPRET(filecheck=CHECK):

interface IDouble { int doubled(); }

struct N { int v; }

extension N : IDouble
{
    int doubled() { return v * 2; }
}

int callDoubled<T : IDouble>(T t) { return t.doubled(); }

void main()
{
    N n; n.v = 5;
    //CHECK: 10
    printf("%d\n", callDoubled(n));
}
