//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#accessordecl-family
//META: doc_section_digest=7ab459ecbfa076cb34ec94ae11dca1b7bd65f74d607addf906138dac14f78530
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
