//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=Two initializers whose parameter types tie for cheapest are both reported as possible while probing and fail with ambiguous conversion when the coerced expression is requested.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the ambiguous-conversion path at an explicit coercion site. A cast
// written on the type name routes through ExplicitCoercion, where nested
// conversions are legal, so float reaches both __init(int) and __init(uint) at
// the same cost; CHECK pins the ambiguity error and one see-declaration note.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct T { int a; __init(int v) { a = 1; } __init(uint v) { a = 2; } }

void main()
{
    float x = 1.0f;
    T t = T(x);
//CHECK:    ^ ambiguous conversion
//CHECK:    ^ more than one conversion exists from 'float' to 'T'
    printf("tag=%d\n", t.a);
}
