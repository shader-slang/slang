//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#extensiondecl
//META: doc_section_digest=05e6c25d2d157e10628b6b3d7185f1f92df372676d332ad265d71884285f0c78
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
