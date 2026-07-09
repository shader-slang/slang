//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#aggregate-collection-types
//META: doc_section_digest=acec388be6a124a34086d58d95713bb5cd33eeac4b8fcf6534173cfc8ec11143
//META: purpose=An enum name is usable as a value type; a variable of the enum type holds one of its enumerators and compares by underlying value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the EnumTypeType claim that an enum type is usable as a
// type name: a variable declared with the enum type holds an
// enumerator, and switching on it selects the matching case. CHECK
// asserts the case body ran, proving the enum acts as a value type.

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
