//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=71d352a8c5034422ec1e8eb2b4a93b52395bd85cc8305cee8432fdeb2101642c
//META: purpose=An enum name is usable as a value type; a variable of the enum type holds one of its enumerators and selects the matching switch case.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an enum declaration yields a usable value type: a local
// declared with the enum name holds an enumerator, and switching on it
// picks that enumerator's case. CHECK asserts the marker written by the
// Green case only, so an enum value that decayed to an untyped integer
// or to the first enumerator would be visible.

//TEST:INTERPRET(filecheck=CHECK):

enum Color { Red, Green, Blue }

void main()
{
    Color c = Color.Green;
    int r = 0;
    switch (c)
    {
    case Color.Red:   r = 1; break;
    case Color.Green: r = 2; break;
    case Color.Blue:  r = 3; break;
    }
    // CHECK: r=2
    printf("r=%d\n", r);
}
