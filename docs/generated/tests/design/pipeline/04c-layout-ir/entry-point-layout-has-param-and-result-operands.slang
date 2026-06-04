//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:02:55+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#per-entry-point-steps
//META: doc_section_digest=88bd1a1de6592fbd4873bdcc84f730186523fdd89ee42e0e9473c34ee75a0278
//META: purpose=lowerEntryPointLayout produces an IREntryPointLayout encoding parameter bindings and stage; the EntryPointLayout(...) instruction in the merged IR takes two varLayout(...) operands (parameters and result).
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Row 7 of the per-entry-point steps table calls
// lowerEntryPointLayout. The resulting IREntryPointLayout
// references both parameter and result varLayouts. In the merged
// IR this appears as `EntryPointLayout(%A, %B)` where %A and %B
// are both defined by `varLayout(...)` instructions earlier in
// the layout block.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### AFTER validateAndRemoveAssumeAddress:
// CHECK: EntryPointLayout(%{{[0-9]+}}, %{{[0-9]+}})
