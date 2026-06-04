//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
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
