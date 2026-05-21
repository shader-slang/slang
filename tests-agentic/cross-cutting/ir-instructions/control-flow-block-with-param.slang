//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#control-flow-instructions
//META: doc_section_digest=b3984feebdaa7987fca66b0f53fdce865b89b787721b062d24304277154e8746
//META: purpose=An `if`/`else` whose two arms each yield a value lowers to a join `block` with a `param` that receives the per-arm value via `unconditionalBranch(merge, value)` — the doc's "block parameters replace SSA `phi`" claim.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc claims `block` is the basic block and that its first N
// children are `param`s, which replace SSA `phi`. A conditional
// assignment (ternary `?:`) is the simplest source-level construct
// that produces this exact pattern: the merge block has a `param`,
// and each branch passes its value as the second operand of
// `unconditionalBranch(merge, value)`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x = (a > 0) ? 1 : -1;
    buf[0] = x;
}

// IR-LABEL: func %main
// IR: ifElse(%{{.*}}, %{{.*}}, %{{.*}}, %{{.*}})
// IR: unconditionalBranch(%{{.*}}, 1 : Int)
// IR: unconditionalBranch(%{{.*}}, -1 : Int)
