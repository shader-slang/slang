//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#getenumbarriermemorytypeflags-and-getenumbarriersemanticflags
//META: doc_section_digest=94b81c5b3ee65db0217000986781d5a66853dd32b46147987fdd5e3c5818dd84
//META: purpose=`getEnumBarrierMemoryTypeFlags` and `getEnumBarrierSemanticFlags` emit HLSL named constants, joining the name of each set bit with a bitwise-or and special-casing the whole-set values `ALL_MEMORY` and `REORDER`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the flag values reach HLSL as identifiers rather than as the
// numbers the IR actually carries, which is the entire reason the two opcodes
// exist. Six `Barrier` calls cover a single bit, a multi-bit set on each
// argument, the all-bits memory value, and the zero-valued semantic; the
// CHECKs are in source order and spell out the exact expected argument text,
// so a baked-in integer or a wrong bit-to-name mapping fails immediately.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main()
{
    Barrier(BarrierMemoryTypeFlags.UAV_MEMORY, BarrierSemanticFlags.GROUP_SYNC);
    Barrier(BarrierMemoryTypeFlags.GROUP_SHARED_MEMORY, BarrierSemanticFlags.DEVICE_SCOPE);
    Barrier(BarrierMemoryTypeFlags.ALL_MEMORY, BarrierSemanticFlags.REORDER);
    Barrier(BarrierMemoryTypeFlags.UAV_MEMORY, (BarrierSemanticFlags)0);
    Barrier(
        BarrierMemoryTypeFlags.UAV_MEMORY |
            BarrierMemoryTypeFlags.NODE_INPUT_MEMORY |
            BarrierMemoryTypeFlags.NODE_OUTPUT_MEMORY,
        BarrierSemanticFlags.GROUP_SCOPE | BarrierSemanticFlags.DEVICE_SCOPE);
    Barrier(
        BarrierMemoryTypeFlags.NODE_INPUT_MEMORY,
        BarrierSemanticFlags.GROUP_SYNC | BarrierSemanticFlags.GROUP_SCOPE);
}

//HLSL: Barrier((UAV_MEMORY), (GROUP_SYNC));
//HLSL: Barrier((GROUP_SHARED_MEMORY), (DEVICE_SCOPE));
//HLSL: Barrier((ALL_MEMORY), (REORDER));
//HLSL: Barrier((UAV_MEMORY), (REORDER));
//HLSL: Barrier((UAV_MEMORY | NODE_INPUT_MEMORY | NODE_OUTPUT_MEMORY), (GROUP_SCOPE | DEVICE_SCOPE));
//HLSL: Barrier((NODE_INPUT_MEMORY), (GROUP_SYNC | GROUP_SCOPE));
