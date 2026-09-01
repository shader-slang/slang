//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#legalizeentrypointsforglsl-despite-the-name
//META: doc_section_digest=3c3c8f77c7e1de8cb96354a3387d70417eaefe0edb55f1e0d96b67d31d068296
//META: purpose=Negative: a semantic the `SV_*` name-match chain does not recognize falls through to error E49999, so an unrecognized system-value semantic fails the SPIR-V compile inside the entry-point legalization rather than at emit.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the fall-through arm of the semantic resolution chain. Every
// recognized SV_* name is mapped to a gl_* builtin identity; a name that
// matches nothing is not silently passed through to the backend but is
// diagnosed while the entry point is being legalized. The caret is anchored
// on the parameter the semantic decorates, which is where the compiler
// reports the span, and the stable code E49999 is pinned rather than the
// message wording.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint bogus : SV_NotARealSemantic)
//CHECK:       ^^^^^ E49999
{
    outBuf[0] = bogus;
}
