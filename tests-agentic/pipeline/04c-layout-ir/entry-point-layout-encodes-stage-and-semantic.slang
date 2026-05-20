//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:02:55+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/pipeline/04c-layout-ir.md#per-entry-point-steps
//META: doc_section_digest=88bd1a1de6592fbd4873bdcc84f730186523fdd89ee42e0e9473c34ee75a0278
//META: purpose=lowerEntryPointLayout encodes parameter bindings and stage into the IREntryPointLayout; the merged IR shows a stage(...) operand inside the entry-point's varLayout and a systemValueSemantic(...) operand for SV_DispatchThreadID.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Row 7 of the per-entry-point steps table says
// "Produces an IREntryPointLayout that encodes parameter bindings
// and stage." When the entry point is compute-staged and takes
// SV_DispatchThreadID, both `stage(...)` and
// `systemValueSemantic(...)` operands appear in the merged layout
// block.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### AFTER validateAndRemoveAssumeAddress:
// CHECK: stage(
// CHECK: systemValueSemantic("SV_DispatchThreadID"
