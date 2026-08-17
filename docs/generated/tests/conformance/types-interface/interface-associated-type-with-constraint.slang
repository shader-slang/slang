//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=An associated type may be constrained to conform to another interface (`associatedtype T : IOther`); the provided type must itself conform to that interface.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a constrained associatedtype (`Elem : IReq`) requires the
// provided concrete type to itself conform to IReq. Owner supplies ElemImpl
// which implements reqValue(); CHECK asserts the call returns 77, proving
// the constraint was satisfied and the method is reachable through the association.

//TEST:INTERPRET(filecheck=CHECK):

interface IReq
{
    int reqValue();
}

interface IOwner
{
    associatedtype Elem : IReq;
    Elem getElem();
}

struct ElemImpl : IReq
{
    int val;
    int reqValue() { return val; }
}

struct Owner : IOwner
{
    typealias Elem = ElemImpl;

    ElemImpl getElem()
    {
        ElemImpl e;
        e.val = 77;
        return e;
    }
}

void main()
{
    Owner o;
    ElemImpl e = o.getElem();
    //CHECK: reqValue=77
    printf("reqValue=%d\n", e.reqValue());
}
