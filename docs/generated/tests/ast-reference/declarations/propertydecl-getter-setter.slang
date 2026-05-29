//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#accessordecl-family
//META: doc_section_digest=1ed46d6cbf110c7915cfeb70b35be1654971c6062c82f1442ee85f909657d1e8
//META: purpose=A PropertyDecl holds GetterDecl/SetterDecl children; reading and writing the property fires the respective accessor.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented roles of PropertyDecl, GetterDecl, and
// SetterDecl together. Writing to the property fires the SetterDecl
// (which doubles the input); reading fires the GetterDecl. The
// observed value reflects both accessors having run.

//TEST:INTERPRET(filecheck=CHECK):

struct Box
{
    int _v;
    property int value
    {
        get { return _v; }
        set { _v = newValue * 2; }
    }
}

void main()
{
    Box b;
    b.value = 7;
    //CHECK: 14
    printf("%d\n", b.value);
}
