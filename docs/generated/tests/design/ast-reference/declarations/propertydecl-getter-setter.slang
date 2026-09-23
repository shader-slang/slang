//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#accessordecl-family
//META: doc_section_digest=8cebf6056445131d843ba65809820ad0e5294659556653bcb7502aa0a4f723ef
//META: purpose=A PropertyDecl holds GetterDecl/SetterDecl children; reading and writing the property fires the respective accessor.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an explicit `get`/`set` pair on a property is what runs on
// read and write. The setter deliberately doubles the incoming value, so the
// CHECK's expected number is only produced when the write went through the
// setter body and the read through the getter body.

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
