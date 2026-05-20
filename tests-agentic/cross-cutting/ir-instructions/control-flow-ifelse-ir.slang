//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#control-flow-instructions
//META: doc_section_digest=b3984feebdaa7987fca66b0f53fdce865b89b787721b062d24304277154e8746
//META: purpose=An `if`/`else` statement lowers to the `ifElse` terminator plus an `unconditionalBranch` join.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An `if/else` body produces three basic blocks (then / else / merge)
// joined by `ifElse` and `unconditionalBranch` — the two terminator
// opcodes the doc lists for conditional and unconditional control
// transfer. `block` and `param` introduce each basic block.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (a > 0)
        buf[0] = 1;
    else
        buf[0] = 2;
}

// IR-LABEL: func %main
// IR: ifElse(%{{.*}}, %{{.*}}, %{{.*}}, %{{.*}})
// IR: unconditionalBranch(%{{.*}})
// IR: unconditionalBranch(%{{.*}})
