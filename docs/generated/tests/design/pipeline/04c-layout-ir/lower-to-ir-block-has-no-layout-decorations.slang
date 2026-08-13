//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#what-this-module-is-not
//META: doc_section_digest=205da7f598ad3d1643fab8a6fb17b6a8f761797e8d8d6da06cb330e073ca2298
//META: purpose=The pre-link executable IR carries no layout decorations and no layout instructions, keeping the per-module IR free of target-specific layout.
//META: intent=negative
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "executable IR is layout-free" property. The pre-link
// `### LOWER-TO-IR:` block is dumped before the layout module is merged,
// so the CHECK-NOT patterns assert that no `[layout(`, `EntryPointLayout(`
// or `structTypeLayout(` text appears between the entry-point function and
// the end of that block.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK: func %main
// CHECK-NOT: [layout(
// CHECK-NOT: EntryPointLayout(
// CHECK-NOT: structTypeLayout(
// CHECK-LABEL: ###
