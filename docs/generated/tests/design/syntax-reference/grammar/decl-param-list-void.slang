//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=c47521b321db82d12ebd0b458b9b12ba738129f46b569b6a37ac0e365a886faa
//META: purpose=`ParamList` accepts the C-compatibility spelling `'void'` as the sole content of the parentheses.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `ParamList ::= 'void'` alternative, which spells an empty
// parameter list the C way. If `void` were parsed as an ordinary Param the
// declaration would want a parameter name after the type, so a call written
// with no arguments could not resolve. Printing 7 proves the parentheses were
// read as an empty list.

//TEST:INTERPRET(filecheck=CHECK):

int seven(void)
{
    return 7;
}

void main()
{
    // CHECK: r=7
    printf("r=%d\n", seven());
}
