//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#control-flow-instructions
//META: doc_section_digest=b3984feebdaa7987fca66b0f53fdce865b89b787721b062d24304277154e8746
//META: purpose=A `discard` statement in a pixel shader lowers to the `discard` IR terminator that the doc lists alongside `return_val` and `unreachable`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `discard` as one of the return / exit terminators of
// the `TerminatorInst` family. A pixel-shader conditional `discard;`
// produces the `discard` opcode in the IR, sitting in the conditional
// arm next to the `unconditionalBranch` join.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage pixel -entry main

uniform int a;

[shader("pixel")]
float4 main() : SV_Target
{
    if (a < 0) discard;
    return float4(1.0);
}

// IR-LABEL: func %main
// IR: discard
