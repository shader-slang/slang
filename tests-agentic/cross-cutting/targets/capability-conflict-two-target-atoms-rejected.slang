//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/targets.md#capability-system
//META: doc_section_digest=a0d40ede85e7cb31a316466bcfe695e496506c5ca59e269adafbe2b4671c8e81
//META: purpose=A `[require(hlsl, glsl)]` conjunction populates the `target` keyhole with two different atoms; the doc says such conjunctions are incompatible and the compiler rejects with `E36111: disallowed capability`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative probe of the doc's Vocabulary rule: "Two conjunctions are
// incompatible if they populate the same keyhole with different atoms
// (e.g. `hlsl + glsl` is incompatible because both populate the
// `target` keyhole)." The `[require(...)]` attribute with both target
// atoms must resolve to the Invalid capability and the compiler must
// reject the require.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

[require(hlsl, glsl)]
/*CHECK:
 ^^^^^^^ disallowed capability
 ^^^^^^^ 'require' resolves into a disallowed `Invalid` Capability.
*/
void helper() {}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    helper();
    outBuf[tid.x] = 1;
}
