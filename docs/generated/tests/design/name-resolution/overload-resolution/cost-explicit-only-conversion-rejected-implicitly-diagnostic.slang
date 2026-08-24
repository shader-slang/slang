//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=kConversionCost_Explicit (90000) is never accepted implicitly, so a non-implicit __init is rejected as an argument conversion and only offered as an explicit one.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the explicit-cast-only level of the cost table. A plain __init
// declares an explicit conversion, so passing an int where the parameter is
// the struct fails the type step; CHECK pins both the type-mismatch error and
// the note that names the conversion as explicit-only.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Boxed { int v; __init(int x) { v = x; } }

int pick(Boxed b) { return b.v; }

void main()
{
    int tag = pick(5);
//CHECK:           ^ type mismatch in expression
//CHECK:           ^ expected an expression of type 'Boxed', got 'int'
//CHECK:           ^ explicit conversion from 'int' to 'Boxed' is possible
    printf("tag=%d\n", tag);
}
