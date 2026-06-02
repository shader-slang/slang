//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=An interface may declare a property requirement with get/set accessors without bodies; the conforming struct provides compatible get and set implementations.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states that properties may be declared in interfaces with
// "get; set;" without implementation bodies, and the conforming type
// provides implementations. Also covers the alternative: a property
// may be implemented by a matching-name variable.

//TEST:INTERPRET(filecheck=CHECK):

interface IHasProp
{
    property val : int
    {
        get;
        set;
    }
}

struct PropWithAccessors : IHasProp
{
    int _val;
    property val : int
    {
        get { return _val; }
        set(int v) { _val = v; }
    }
}

void main()
{
    PropWithAccessors obj;
    obj.val = 55;
    //CHECK: val=55
    printf("val=%d\n", obj.val);
}
