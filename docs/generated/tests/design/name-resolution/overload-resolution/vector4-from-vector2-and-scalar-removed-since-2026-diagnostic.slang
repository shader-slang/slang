//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#conversions-that-do-not-have-their-own-cost-level
//META: doc_section_digest=ecd3a6f3ea79e7ef3d125546849e46357976725d2a9d4de69b69741a1f69fac4
//META: purpose=Compiling with language version 2026 turns the same float4(f2, f) call into an error because [RemovedSince] supersedes the deprecation message.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the language-version boundary the doc names for the legacy
// three-component vector initializer. The directive selects `-std 2026`, past
// the declaration's RemovedSince version; CHECK pins the removed-declaration
// error in place of the warning the default version reports.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-std 2026

float4 makeFour(float2 f2, float f)
{
    return float4(f2, f);
//CHECK:   ^^^^^^ has been removed since language version '2026'
}

void main()
{
    float4 v = makeFour(float2(1.0f, 2.0f), 9.0f);
    printf("v=%d\n", int(v.z));
}
