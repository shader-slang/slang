//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=e5426f9afa17becb9e7d33c23301014e082fad12bbf8aff1c4a169d90575556e
//META: purpose=Assigning a float[2] to a float[4] reports type-mismatch E30019, not the similarly-named cannot-convert-array-of-smaller-to-larger-size entry E30024.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The converse half of the "guessing an input from an entry's name is
// unreliable" claim. `cannot-convert-array-of-smaller-to-larger-size`
// reads like the diagnostic for exactly the assignment written below,
// but that entry belongs to varying legalization for an oversized
// system-value array; a plain assignment goes through ordinary
// conversion and reports a type mismatch instead. The test performs the
// assignment and asserts E30019 fires while E30024 does not, so a change
// that made the name-suggested entry fire here would fail rather than
// silently pass.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float a[2] = { 1.0, 2.0 };
    float b[4] = a;
    outBuf[tid.x] = b[0];
}

// CHECK-NOT: E30024
// CHECK: E30019
// CHECK-NOT: E30024
