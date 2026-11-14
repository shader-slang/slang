//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=c47521b321db82d12ebd0b458b9b12ba738129f46b569b6a37ac0e365a886faa
//META: purpose=`PropertyDecl` also accepts the traditional C form `'property' Type Declarator AccessorBlock`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second PropertyDecl arm, which puts the type before the name
// instead of after a colon. The two arms are distinguished only by token
// order, so the test declares `property int val` and reads it; printing 17
// from a backing field of 16 proves the traditional spelling bound a property
// whose getter ran, not a plain field named `val`.

//TEST:INTERPRET(filecheck=CHECK):

struct S
{
    int store;

    property int val
    {
        get { return store + 1; }
    }
}

void main()
{
    S s;
    s.store = 16;
    // CHECK: r=17
    printf("r=%d\n", s.val);
}
