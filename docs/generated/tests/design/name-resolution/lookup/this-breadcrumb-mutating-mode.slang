//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=d1c02da07b357c8b41134fbb13a153c9451e991db3087032b3d340a5ddfdfa75
//META: purpose=A [mutating] method's implicit this is writable (the modifier matters to the checker's this computation, not to lookup), so an unqualified write to a field inside that method updates the receiver.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a [mutating] method yields a writable implicit `this`:
// lookup records a ThisExpr breadcrumb and the checker's expansion of
// it reads the function's effective this-parameter information. The
// method assigns to `count` by unqualified name; if `this` were
// immutable the write would be rejected or land on a copy. CHECK asserts the receiver observed both increments.

//TEST:INTERPRET(filecheck=CHECK):

struct Counter
{
    int count;
    [mutating] void bump() { count = count + 1; }
}

void main()
{
    Counter c;
    c.count = 10;
    c.bump();
    c.bump();
    // CHECK: r=12
    printf("r=%d\n", c.count);
}
