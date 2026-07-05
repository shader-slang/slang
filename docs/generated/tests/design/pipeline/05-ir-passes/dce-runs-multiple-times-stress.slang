//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#how-the-passes-are-ordered
//META: doc_section_digest=c06c7e69cd34636d41e63eeea18fb6cc8b82ab5dcea4b909122919abf39cf05a
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
