//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=At an implicit site disallowNestedConversions demands an exact parameter match, so the same two-initializer type reports an ordinary type mismatch rather than an ambiguity.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the implicit-site half of the ambiguous-conversion example. An
// initialization without a cast is not an ExplicitCoercion site, so neither
// __init matches float exactly and at most one initializer can apply; CHECK
// pins the ordinary type-mismatch error rather than an ambiguity.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct T { int a; __init(int v) { a = 1; } __init(uint v) { a = 2; } }

void main()
{
    float x = 1.0f;
    T t = x;
//CHECK:  ^ type mismatch in expression
//CHECK:  ^ expected an expression of type 'T', got 'float'
    printf("tag=%d\n", t.a);
}
