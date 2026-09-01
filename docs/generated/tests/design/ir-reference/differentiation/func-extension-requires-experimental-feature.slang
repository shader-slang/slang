//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#synthesized-derivative-witnesses
//META: doc_section_digest=4ac46248bc67dd90a58b5dbd80b92929f4d02dd326a0ef7823411d0e2a772c21
//META: purpose=The __func_extension surface behind the IdentityRemat row is experimental and is rejected in user code unless -experimental-feature is passed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the gate on the only user surface that produces `IdentityRemat`.
// The companion test `identity-remat.slang` compiles the same shape *with*
// `-experimental-feature`; this file compiles it without, and asserts that
// checking rejects the declaration. The caret is on the `__apply` token of the
// `__func_extension` declaration and carries E30131, which is what pins the
// gate to the declaration itself rather than to the later use — the entry
// point still compiles, so the diagnostic is the only evidence the extension
// was dropped.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

uniform float a;
uniform RWStructuredBuffer<float> buf;

float sqr(float x) { return x * x; }

struct SqrContext
{
    float cached;
    void operator()(out float dx, float dOut) { dx = dOut * cached; }
};

__func_extension __apply(sqr)(float x) -> Tuple<float, SqrContext>
//CHECK:         ^^^^^^^ E30131
//CHECK:         ^^^^^^^ __func_extension declarations are rejected unless '-experimental-feature' is enabled.
{
    return makeTuple(x * x, SqrContext(2.0 * x));
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = sqr(a);
}
