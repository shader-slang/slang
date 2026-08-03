//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:27:51Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#what-this-module-is-not
//META: doc_section_digest=98e652ccdabfc883b1140a71a392d5b79042be133038ba455d1932a654abc8e2
//META: purpose=linkIR pulls the existing layout module into its symbol set, so IRLayoutDecorations absent from the pre-link executable IR become visible on the linked global param and entry point in the first post-link snapshot.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that layout decorations reach the linked IR only via linkIR
// merging the layout module into its symbol set. The CHECK-NOTs pin the
// pre-link ### LOWER-TO-IR: region as layout-free; the CHECKs after the
// first post-link ### AFTER ...: label then find [layout(...)] on the
// global param stub and an EntryPointLayout instruction — decorations
// that can only have arrived through the merged layout module, since no
// pre-link pass attaches them.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-LABEL: ### LOWER-TO-IR:
// CHECK-NOT: [layout(
// CHECK-NOT: EntryPointLayout(
// CHECK-LABEL: ### AFTER validateAndRemoveAssumeAddress:
// CHECK: EntryPointLayout(
// CHECK: [layout(%{{[0-9]+}})]
// CHECK: global_param
