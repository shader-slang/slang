//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=A property requirement in an interface may alternatively be satisfied by declaring a compatible variable with a matching name in the conforming struct.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states "Alternatively, a property may be implemented by declaring
// a compatible variable with a matching name."
// Verifies that a plain data member with the same name satisfies a
// property requirement.

//TEST:INTERPRET(filecheck=CHECK):

interface IHasFloat
{
    property myVal : float
    {
        get;
        set;
    }
}

struct StructWithField : IHasFloat
{
    // Plain field satisfies the property requirement
    float myVal;
}

void main()
{
    StructWithField s;
    s.myVal = 3.5f;
    //CHECK: myVal=3.5
    printf("myVal=%.1f\n", s.myVal);
}
