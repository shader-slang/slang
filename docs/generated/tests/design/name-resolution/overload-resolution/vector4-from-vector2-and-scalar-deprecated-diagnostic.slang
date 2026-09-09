//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversions-that-do-not-have-their-own-cost-level
//META: doc_section_digest=ecd3a6f3ea79e7ef3d125546849e46357976725d2a9d4de69b69741a1f69fac4
//META: purpose=The (vector<T,2>, T) initializer overload resolution binds to is marked [deprecated], so a float4(f2, f) call reports the deprecation warning.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the declaration overload resolution binds to for float4(f2, f)
// is the deprecated one rather than a scalar-to-vector promotion. CHECK pins
// the deprecation warning, which only fires if the [deprecated] declaration is
// the selected candidate.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

float4 makeFour(float2 f2, float f)
{
    return float4(f2, f);
//CHECK:   ^^^^^^ has been deprecated
}

void main()
{
    float4 v = makeFour(float2(1.0f, 2.0f), 9.0f);
    printf("v=%d\n", int(v.z));
}
