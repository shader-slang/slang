//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#emitting-hlsl-named-constants-rather-than-integers
//META: doc_section_digest=788bc7519655c8e116120260bbcac10f921c9993b6dbc20665b3dec1d2d85f1b
//META: purpose=A work-graph `Barrier(...)` flag set emits as HLSL named constants — the all-bits shorthand `(ALL_MEMORY)` / `(REORDER)` and a `|`-joined per-bit list — never as the integer behind the name.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the HLSL backend reconstructs the *name* of a barrier flag
// set instead of baking in the integer DXC happens to use for it today.
// The test issues two `Barrier(...)` calls: one whose memory-type operand is
// exactly the all-bits value and whose semantic operand is the zero-valued
// `REORDER`, and one whose operands are genuine multi-bit combinations.
// CHECK asserts the first emits the all-bits shorthand `(ALL_MEMORY)` /
// `(REORDER)` and the second emits a `|`-joined per-bit list, while the
// interleaved CHECK-NOT lines assert the numeric spellings (`15` for
// ALL_MEMORY, `5` for UAV_MEMORY|NODE_INPUT_MEMORY) never appear — which is
// the whole point of routing the value through the emit-side name table.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature

import experimental.workgraph;

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main()
{
    Barrier(BarrierMemoryTypeFlags.ALL_MEMORY, BarrierSemanticFlags.REORDER);
    Barrier(
        BarrierMemoryTypeFlags.UAV_MEMORY | BarrierMemoryTypeFlags.NODE_INPUT_MEMORY,
        BarrierSemanticFlags.GROUP_SYNC | BarrierSemanticFlags.DEVICE_SCOPE);
}

// The all-bits value takes the shorthand spelling, not a four-way OR list.
// CHECK-NOT: Barrier(15
// CHECK: Barrier((ALL_MEMORY), (REORDER));
// A genuine multi-bit set becomes a `|`-joined list of per-bit names.
// CHECK-NOT: Barrier(5,
// CHECK: Barrier((UAV_MEMORY | NODE_INPUT_MEMORY), (GROUP_SYNC | DEVICE_SCOPE));
