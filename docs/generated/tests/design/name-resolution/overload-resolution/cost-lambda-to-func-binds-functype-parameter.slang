//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversion-costs
//META: doc_section_digest=ac2c2ef8658efb348430e3e014feb776a1624731934544b00f6fa4d97ab710e8
//META: purpose=kConversionCost_LambdaToFunc (1) makes a lambda bind to a functype parameter, so the lambda body is the callee the parameter invokes.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lambda-to-Func row of the cost table. The lambda is written at
// the call site and the parameter is declared with `functype`, so the
// conversion the ranking charges 1 for is the only way the call can typecheck;
// CHECK asserts the product the lambda computes.

//TEST:INTERPRET(filecheck=CHECK):

int apply(functype(int, int) -> int f, int a, int b) { return f(a, b); }

void main()
{
    int tag = apply((int x, int y) => x * y, 3, 4);
    // CHECK: tag=12
    printf("tag=%d\n", tag);
}
