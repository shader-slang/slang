//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#control-flow-instructions
//META: doc_section_digest=0b68b77b6156c67db4c587635b263df5f1c7f5c3d702914334c92fc6b5e88c90
//META: purpose=An `if`/`else` statement lowers to the `ifElse` terminator plus `unconditionalBranch` joins.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's control-flow terminator rows. An `if/else` produces
// then/else/merge basic blocks joined by the `ifElse` terminator and
// `unconditionalBranch` back-edges to the merge block, matching the
// `TerminatorInst` family entries.

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
