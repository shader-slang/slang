//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=c47521b321db82d12ebd0b458b9b12ba738129f46b569b6a37ac0e365a886faa
//META: purpose=`AccessorDecl` accepts an optional `'(' ParamList? ')'`, which is how a `set` accessor names the incoming value.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the optional parameter-list slot on AccessorDecl. The `set`
// accessor names its incoming value `newValue` and doubles it before storing,
// so the name has to be in scope inside the accessor body for the program to
// compile at all. Assigning 8 and reading back 16 proves the named parameter
// carried the assigned value rather than some implicit binding.

//TEST:INTERPRET(filecheck=CHECK):

struct S
{
    int store;

    property val : int
    {
        get { return store; }
        set(newValue) { store = newValue * 2; }
    }
}

void main()
{
    S s;
    s.val = 8;
    // CHECK: r=16
    printf("r=%d\n", s.val);
}
