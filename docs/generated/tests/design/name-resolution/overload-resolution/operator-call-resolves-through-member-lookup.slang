//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#the-general-path
//META: doc_section_digest=fcdc0ef5f61233fa8b4fb6def9878dcee51b42c2d1bbde085f283c94b50f6fdd
//META: purpose=operator() is resolved by member lookup on the callee's type, so calling a struct value like a function dispatches to its operator() member.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the member-lookup half of operator dispatch. Unlike infix
// operators, the call form looks operator() up on the callee value's type and
// keeps the implicit this, so the member sees the receiver's field; CHECK
// asserts a result that combines the stored base with the call argument.

//TEST:INTERPRET(filecheck=CHECK):

struct Adder
{
    int base;
    int operator()(int x) { return base + x; }
}

void main()
{
    Adder a;
    a.base = 10;
    int tag = a(5);
    // CHECK: tag=15
    printf("tag=%d\n", tag);
}
