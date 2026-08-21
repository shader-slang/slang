//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#decorations
//META: doc_section_digest=af2eb87d563403d52d71a7d33122c279b70c033339e51393dbe00f9f5f506a59
//META: purpose=A core-module intrinsic reached from user code carries a `targetIntrinsic` decoration mapping the IR function to its target spelling.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `targetIntrinsic`/`TargetIntrinsicDecoration` row and its
// `target, definitionOperand` operand shape. Calling
// `GroupMemoryBarrierWithGroupSync()` drags the core-module declaration into
// the module; the IR CHECK asserts the imported `func` carries a
// `targetIntrinsic(...)` decoration whose definition operand is the target
// spelling as a string. The HLSL CHECK closes the loop by showing that
// spelling is what reaches the emitted code, so the decoration is the
// IR-to-target mapping the row describes rather than inert metadata.

//TEST:SIMPLE(filecheck=IR):-target hlsl -dump-ir -o /dev/null -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

groupshared uint sdata[4];
uniform RWStructuredBuffer<uint> outbuf;

[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, uint gi : SV_GroupIndex)
{
    sdata[gi] = gi;
    GroupMemoryBarrierWithGroupSync();
    outbuf[gi] = sdata[3 - gi];
}

// IR: [targetIntrinsic(%{{.*}}, "GroupMemoryBarrierWithGroupSync")]
// IR: func %GroupMemoryBarrierWithGroupSync

// HLSL: GroupMemoryBarrierWithGroupSync();
