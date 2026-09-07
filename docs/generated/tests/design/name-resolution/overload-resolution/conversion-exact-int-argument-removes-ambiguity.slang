//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=45a19e6a6506ddb836a3ebca70fa59e866619b52b9904fc36ec1843e2627f208
//META: purpose=An int argument to the same two-initializer type removes the ambiguity because the exact match costs 0.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's stated escape from the ambiguous-conversion example.
// Swapping the float argument for an int gives __init(int) an exact match at
// cost 0, which strictly beats __init(uint); CHECK asserts the tag stored by
// the int initializer.

//TEST:INTERPRET(filecheck=CHECK):

struct T { int a; __init(int v) { a = 1; } __init(uint v) { a = 2; } }

void main()
{
    int x = 7;
    T t = T(x);
    // CHECK: tag=1
    printf("tag=%d\n", t.a);
}
