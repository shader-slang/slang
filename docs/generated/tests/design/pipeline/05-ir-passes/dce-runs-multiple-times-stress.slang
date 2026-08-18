//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#how-the-passes-are-ordered
//META: doc_section_digest=66d40862801105520d0f443657f6c0a7e1b9289bce092153a8e9060d2f1b77f5
//META: purpose=Stress: the eliminateDeadCode pass runs at least three times during the SPIR-V pipeline — three or more `### AFTER eliminateDeadCode:` headers appear in the dump.
//META: intent=stress
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Extension of `eliminate-dead-code-runs-multiple-stages.slang`:
// confirm three or more invocations of eliminateDeadCode are
// visible. The orchestrator interleaves cleanup throughout the
// post-link pipeline.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir-after eliminateDeadCode -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + 1;
}

// CHECK: ### AFTER eliminateDeadCode:
// CHECK: ### AFTER eliminateDeadCode:
// CHECK: ### AFTER eliminateDeadCode:
