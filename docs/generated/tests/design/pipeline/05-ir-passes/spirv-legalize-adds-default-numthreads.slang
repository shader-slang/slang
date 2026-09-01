//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling
//META: doc_section_digest=5ecc062dc93859b69b080811db13201d5e12052d67767a4d49a4d8787cb40272
//META: purpose=The passes in between only read the numthreads decoration, with one exception: SPIR-V legalization adds a default NumThreadsDecoration of (1, 1, 1) to a compute entry point that has none.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the single documented exception to "no pass in this category
// lowers `[numthreads(...)]`". The entry point below deliberately carries
// no `[numthreads]` attribute at all, so nothing in the front end can have
// supplied a launch dimension. The CHECK asserts SPIR-V still emits an
// explicit `LocalSize 1 1 1` execution mode, which can only come from the
// default decoration the SPIR-V legalizer synthesizes.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main

RWStructuredBuffer<int> outBuf;

[shader("compute")]
void main(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = int(tid.x);
}

// SPIRV: OpExecutionMode %main LocalSize 1 1 1
