//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
//META: purpose=An AssocTypeDecl declares an associated type in an interface; the conforming type supplies a concrete typealias and dispatch produces that type's value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of AssocTypeDecl. The interface
// declares `associatedtype Item`; the conforming struct supplies a
// concrete `Item = int`; an interface method using the associated
// type dispatches and returns the concrete value.

//TEST:INTERPRET(filecheck=CHECK):

interface IContainer
{
    associatedtype Item;
    Item first();
}

struct IntStream : IContainer
{
    typealias Item = int;
    int firstVal;
    Item first() { return firstVal; }
}

IntStream.Item peek(IntStream s) { return s.first(); }

void main()
{
    IntStream s; s.firstVal = 11;
    //CHECK: 11
    printf("%d\n", peek(s));
}
