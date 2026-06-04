//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup
//META: doc_section_digest=bf314e51995b5a2b76e1104cf5f719db98f30e41b3a1ec2cddb5792a8ffcdf27
//META: purpose=The eliminate-phis pass converts the SSA form into a non-SSA form ahead of emit; the final IR-dump stage `### AFTER eliminatePhis:` exists and contains `func %main`.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `slang-ir-eliminate-phis.cpp` is listed in the "SSA construction
// and basic cleanup" category. It converts SSA out at the very end
// of the pipeline. The `### AFTER eliminatePhis:` header is the
// observable footprint.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir-after eliminatePhis -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x;
    if (a > 0) { x = a + 1; } else { x = a - 1; }
    buf[0] = x;
}

// CHECK-LABEL: ### AFTER eliminatePhis:
// CHECK: func %main
